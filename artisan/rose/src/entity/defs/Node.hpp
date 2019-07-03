#pragma once

#include <algorithm>
#include <list>
#include <boost/range/algorithm_ext/erase.hpp>

#include <entity/Entity.hpp>
#include <utils/rose_utils.hpp>
#include <utils/misc_utils.hpp>
#include <py_fns/py_query.hpp>

ENTITY_SPEC_BEGIN(Node, "generic AST node", SgNode, Entity, node, obj, entity, sg_type) 
{
    bind_attr(obj, "entity", "class name", entity); 

    bind_attr(obj, "entities", "entities", 
         [entity] () -> py::list {
              EntityInfo *info = EntityManager::expect_entity(entity);
              std::list<std::string> &entities = info->meta_entities;
              py::list lst = py::list(entities);
              return lst;
         }());

    bind_attr(obj, "uid", "unique identifier (within a model)", (int64_t) node);   
    bind_attr(obj, "tag", "tag", py::str((int64_t) node));
    bind_attr(obj, "sg_type", "ir type", sg_type); 
    bind_attr(obj, "sg_type_real", "ir type", (const char *) node->sage_class_name());                   

    bind_method(obj, "is_entity", "blah", is_entity);

    bind_method(obj, "project", "", project);
    bind_method(obj, "parent", "", parent);
    bind_method(obj, "children", "my children - node", children);
    bind_method(obj, "num_children", "", num_children);
    bind_method(obj, "tree", "",
                    tree, 
                    (ARG("depth", -1), 
                    ARG("only_src", true)));  

    bind_method(obj, "query", "beautiful query", 
                    query, 
                    (ARG("match"), 
                    ARG("where", ""), 
                    ARG("env", boost::python::object())));
    
    bind_method(obj, "unparse", "", 
                        unparse, 
                        (ARG("updates") = true, 
                        ARG("format")=1));  
    bind_method(obj, "traverse", "", 
                    traverse, 
                    (ARG("pre")=py::object(), 
                    ARG("arg")=py::object(), 
                    ARG("env")=py::object(), 
                    ARG("post")=py::object()));               
}

static bool is_entity(py::object self, std::string name) {
    SgNode *node = to_sgnode(self);

    std::string entity = EntityManager::expect_sg_entity(node->sage_class_name());
    std::list<std::string> entities = EntityManager::get_entity(entity)->meta_entities;
    std::list<std::string>::iterator i = std::find(entities.begin(), entities.end(), name);
    return i != entities.end();
}

static SgNodePtr project(SgNodePtr self) {
    SgNode *node = self;
    PrjAttribute *attr = (PrjAttribute *) node->getAttribute("project");
    if (attr) {
        return attr->prj();
    } else {
        return 0;
    }
}

static SgNodePtr parent(SgNodePtr self) {
    SgNode *node = self;
    return node->get_parent();
}

static std::list<SgNodePtr> children(SgNodePtr self) {
    SgNode *node = self;
    std::list<SgNodePtr> lst;
    size_t n = node->get_numberOfTraversalSuccessors();
    for (size_t i = 0; i < n; i++) {
        SgNode *child = node->get_traversalSuccessorByIndex(i);
        lst.push_back(child);
    }
    
    return lst;
}   

static int64_t num_children(py::object obj) {
    SgNode *node = (SgNode *) to_sgnode(obj);  
    return node->get_numberOfTraversalSuccessors();
}   

static std::string unparse(py::object self, bool updates, int format) {
    SgNode *node = (SgNode *) to_sgnode(self);

    // 0 => raw from ROSE's unparser
    // 1 => bcpp
    // 2 => token remove leading/trailing spaces
    // 3 => token remove leading/trailing spaces including ';'

    bool pretty_print = (format == 1);

    std::string code = RoseUtils::unparse_code(node, updates, pretty_print);

    switch (format) {
        case 2: code = MiscUtils::str_trim(code, " \t\n"); break;       
        case 3: code = MiscUtils::str_trim(code, " \t\n;"); break;

    }

    return code;
}

static std::string tree(py::object obj, int max_depth, bool only_src) {
    
    struct TreeVisitor: public AstTopDownProcessing<int> {
    std::string _str;
    int _max_depth;  
    bool _only_src;

    TreeVisitor(int max_depth, bool only_src) {
        _max_depth = max_depth;
        _only_src = only_src;
    }

    int evaluateInheritedAttribute(SgNode *node, int depth) {     
        //if (_blacklist.find(node->class_name()) != _blacklist.end()) return depth; 
        if (_max_depth == -1 || depth <= _max_depth) {
            py::object pynode = create_rose_node(node);
            if (!_only_src || (isSgLocatedNode(node) && (bool) boost::python::extract<bool>(pynode.attr("in_src")()))) {                                        
                _str += std::string(depth*3, '.') +  (std::string) boost::python::extract<std::string>(pynode.attr("__repr__")());
            }  
        }   
        
        return depth + 1;
    }     
    };

    SgNode *node = (SgNode *) to_sgnode(obj);
    TreeVisitor tv(max_depth, only_src);
    tv.traverse(node, 0);
    return tv._str;
}

static py::object query(py::object root, std::string match, std::string condition, py::object env) {
    return __query(root, match, condition, env);
}

static py::object traverse(py::object root, py::object pre, py::object arg, py::object env, py::object post) {
    //if (arg == py::object()) { pre(); return py::object(); } else { return pre(arg); }
    
    struct Traversal: public SgTopDownBottomUpProcessing<py::object, py::object> {
        py::object _pre;
        py::object _post;
        py::object _env;
        size_t _pre_nargs;
        size_t _post_nargs;

        Traversal(py::object pre, py::object post, py::object env) {
            _pre = pre; _post = post; _env = env;
            if (_pre == py::object()) {
                _pre_nargs = 0;
            } else {
                _pre_nargs = boost::python::extract<std::size_t>(_pre.attr("__code__").attr("co_argcount"));
            }

            if (_post == py::object()) {
                _post_nargs = 0;
            } else {
                _post_nargs = boost::python::extract<std::size_t>(_post.attr("__code__").attr("co_argcount"));
                set_useDefaultIndexBasedTraversal(false);
            }
            
        }
        void setNodeSuccessors(SgNode* node, SuccessorsContainer& succContainer) {               
            succContainer = node->get_traversalSuccessorContainer();
            boost::remove_erase(succContainer, (SgNode *) NULL);
        }
        py::object evaluateInheritedAttribute(SgNode *node, py::object arg) {
            if (_pre == py::object()) return py::object();
            py::object pynode = create_rose_node(node);  

            if (_pre_nargs <= 1) {
                return _pre(pynode);
            } else if (_pre_nargs == 2) {
                return _pre(pynode, arg);
            } else {
                return _pre(pynode, arg, _env);
            }
        }
        py::object evaluateSynthesizedAttribute(SgNode *node, py::object arg, SynthesizedAttributesList synth_attrs) {
            if (_post == py::object()) return py::object();
            py::object pynode = create_rose_node(node);  
            py::list lst = py::list();
            for (auto i: synth_attrs) {                      
                lst.append(i);
            }               

            if (_post_nargs <= 1) {
                return _post(pynode);
            } else if (_post_nargs <= 3) {
                return _post(pynode, arg, lst);
            } else {
                return _post(pynode, arg, lst, _env);
            }
        }
    };

    Traversal t(pre, post, env);
    SgNode *node = (SgNode *) to_sgnode(root);
    t.traverse(node, arg);
    return t._env;

}
  
ENTITY_SPEC_END