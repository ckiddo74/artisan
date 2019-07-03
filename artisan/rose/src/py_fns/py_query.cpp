#include <rose.h>
#include "py_query.hpp"
#include <list>
#include <string>
#include <regex>

#include "query_grammar.hpp"
#include <py_sgnode.hpp>
#include <entity/EntityManager.hpp>
#include <utils/hmsg.hpp>

using namespace std;
namespace py = boost::python;
namespace rqg = rose_query_grammar;

bool eval_condition(string condition, const list<string> &vars, const list<SgNode *> &nodes, py::object env) {
    
    py::dict scope = py::dict();

    // add external variables first
    if (env != py::object()) {
       py::dict d = py::extract<py::dict>(env);
       scope = d.copy();
    }   

    // add query objects into scope
    list<string>::const_iterator it = vars.begin();
    for (auto &node : nodes) {
       scope[(*it)] = create_rose_node(node);
       it++;
    }   
           
    py::object res = py::eval(condition.c_str(),  scope);
     
    return py::extract<bool>(res);
} 

bool eval_tag(string tag, SgNode *node) {

   if (tag.empty()) return true;
   
   py::object obj = create_rose_node(node);
   string obj_tag = py::extract<string>(obj.attr("tag"));
   //printf(":TAG:> %s, cond_tag: %s\n", obj_tag.c_str(), tag.c_str());
   
   smatch m;
   
   regex re(tag); 
   
   return regex_match(obj_tag, m, re);
}

class QueryVisitor: public AstTopDownProcessing<int> {
protected:
   rqg::query_node_t _qryent_h;
   list<rqg::query_node_t> _qryent_r;
   
   int _min_depth;
   int _max_depth;
   list<rqg::query_edge_t> _qrydist_r;
   
   bool _depth_constraint;
   
   SgNode *_root;
   string _condition;   
   list<list<SgNode *>> _matches;
   list<string> _vars;  

   string _entity, _tag;
   py::object _env;

      
public:
	QueryVisitor(SgNode *root, const list<rqg::query_node_t> &queryEnt, const list<rqg::query_edge_t> &queryDist, string condition, py::object env) {
        _qryent_h = queryEnt.front();
        _qryent_r = queryEnt; _qryent_r.pop_front();       
        
        rqg::query_edge_t qrydist_h = queryDist.front();
        _min_depth = qrydist_h.min;
        _max_depth = qrydist_h.max;
        _depth_constraint = _max_depth > 0;
        
        _entity = _qryent_h.entity;
        _tag    = _qryent_h.tag;
        
        _qrydist_r = queryDist; _qrydist_r.pop_front();
        
        _root = root; 
        _condition = condition;
        _env = env;
        
        if (!condition.empty()) {
            for (auto &q : queryEnt) {
                string var;
                var = q.var;
                if (var != "") {
                    _vars.push_back(var);
                }   
                
            }            
        }
    }  
   
    const list<list<SgNode *>>& matches() { 
        return _matches;
    }   

    void process_node(SgNode *node) {
        EntityInfo *entity_info = EntityManager::get_entity(_entity);             
        hAssert(entity_info, "Query error: entity '%s' is not valid!", _entity);

        if ((*entity_info->check_fn)(node) ) { // checks entity
            if (eval_tag(_tag, node)) { // check tag	                
                list<list<SgNode *>> lsts;            
                if (_qryent_r.empty()) { // base
                    lsts.push_back({node});
                    //printf("...reached base!\n");
                } else {
                    size_t n = node->get_numberOfTraversalSuccessors();
                    for (size_t i = 0; i < n; i++) {
                        SgNode *child = node->get_traversalSuccessorByIndex(i);
                        if (child) {
                            QueryVisitor qv_child (child, _qryent_r, _qrydist_r, "", _env);    
                            qv_child.run(1);     
                            const list<list<SgNode *>> &child_matches = qv_child.matches();
                            for (auto &l : child_matches) {
                                list<SgNode *> lst;
                                lst = l;
                                lst.push_front(node);
                                lsts.push_back(lst);
                            }                        
                        }                                   
                    }             
                }                
                for (auto &lst: lsts) {
                    // check condition
                    if (_condition.empty() || eval_condition(_condition, _vars, lst, _env)) {
                        _matches.push_back(lst);      
                    }      
                }                 
            }   
        }
    }
    
    int evaluateInheritedAttribute(SgNode *node, int depth) {  
       //printf("QUERY: %s, DEPTH: %d, MIN_DEPTH: %d, MAX_DEPTH: %d\n", node->sage_class_name(), depth, _min_depth, _max_depth);    
       if (!_depth_constraint || (_min_depth <= depth && _max_depth >= depth)) {
           process_node(node);
       }
       return depth + 1;  
    }
    void run(int depth) {
       if (!_depth_constraint) {
           enum VariantT variant;

           EntityInfo *entity_info = EntityManager::get_entity(_entity);             
           hAssert(entity_info, "Query error: entity '%s' is not valid!", _entity);
           
           variant = entity_info->sg_variant;
           vector<SgNode *> nodes = NodeQuery::querySubTree(_root, variant, AstQueryNamespace::AllNodes);   
           //printf("%d, %d\n", (int) variant, nodes.size() );
           for (auto node : nodes) {
               process_node(node);
           }
       } else {
           traverse(_root, depth);
       }
    }

};


py::list create_query_results(const list<rqg::query_node_t> &entities, const list<list<SgNode *>> &matches) {
   py::list ret = py::list();
     
   for (auto &l : matches) {
     list<rqg::query_node_t>::const_iterator it = entities.begin();
     py::dict entry = py::dict();
     for (auto node : l) {
        string var = (*it).var;
        if (var != "") {
           entry[var] = create_rose_node(node);
        }   
        it++;
     }
     ret.append(entry);
   }  
   return ret;
}



py::object __query(py::object root, std::string match, string condition, py::object env) {
 
    rqg::query_t query;
    rqg::parse(match, query);
    rqg::print(query);

    SgNode *rnode = (SgNode *) to_sgnode(root);

    list<rqg::query_node_t> &nodes = query.nodes;
    list<rqg::query_edge_t> &edges = query.edges;

    list<list<SgNode *>> matches;
    QueryVisitor qv(rnode, nodes, edges, condition, env);
    qv.run(0); 
    matches = qv.matches();

    py::dict results = py::dict();
    py::list res_nodes = create_query_results(query.nodes, matches);

    py::list keys = py::list();
    for (auto &node : nodes) {
        if (node.var != "") {
            keys.append(node.var);
        }   
    }

   results["nodes"] = res_nodes;
   results["keys"] = keys;
   py::object mod = py::import("artisan.rose.table");
   py::object table = mod.attr("RoseNodeTable")(results);

   return table;
} 
