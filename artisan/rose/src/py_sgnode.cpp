#include <py_sgnode.hpp>
#include <utils/hmsg.hpp>
#include <entity/EntityManager.hpp>
#include <utils/htimer_log.hpp>

using namespace std;

namespace py = boost::python;

map<SgProject *, PrjAttribute *> project_attributes;
map<SgProject *, list<py::object> > rose_objs;

void prepare_project(SgProject *project) {

  // qualification name for C++
   void generateNameQualificationSupport( SgNode* node, std::set<SgNode*> & referencedNameSet );  
   for (int i = 0; i < project->numberOfFiles(); i++) {
       SgSourceFile *f = isSgSourceFile(&(project->get_file(i)));
       std::set<SgNode*> referencedNameSet; 
       generateNameQualificationSupport(f,referencedNameSet);
   }

   // annotate all nodes with link to their project
   vector<SgNode *> nq = NodeQuery::querySubTree(project, V_SgNode, AstQueryNamespace::AllNodes);  
   PrjAttribute *attr = new PrjAttribute(project);
   for (auto node : nq) {
      node->setAttribute("project", attr);
   }
   project_attributes[project] = attr;
}

/************************************************************************[ python interface ]****/
py::object __frontend(list<string> args) {
   size_t n = args.size();
   size_t argc = n + 1;
   
   const char *argv[argc];
   
   argv[0] = "";
   
   size_t j = 0;
   for (auto &arg : args) {
      argv[j+1] = arg.c_str();
      j++;
   }

   SgProject *project;

   {
      tlog(1, "parser", "ROSE frontend: %s",  args);
      project = frontend(argc, (char **) argv);
   } 
   prepare_project(project);

   py::object obj = create_rose_node(project);

   return obj;   
} 

void __destroy_project(py::object prj) {
   assert_entity(prj, "Project");       

   SgProject *project = (SgProject *) to_sgnode(prj);

   //printf(":::::> Destroying project: %p\n", project);

   

   if (rose_objs.find(project) != rose_objs.end()) {
      for (auto obj: rose_objs[project]) {
          //printf("::::> %p => %ld\n", obj.ptr(), Py_REFCNT(obj.ptr()));
          Py_DecRef(obj.ptr());
      }
      rose_objs.erase(project);
   }

   
   //PrjAttribute *prj_attr = project_attributes.at(project);
   //printf("%p\n", prj_attr);
   //delete prj_attr;
   project_attributes.erase(project);
   

 
 /* Not sure how to remove all nodes without corrupting the memory pool !
   class DeleteAST : public AstSimpleProcessing {
     list<SgNode *> _ns;
     public:
        void visit (SgNode* node) { if (!isSgTypedefDeclaration(node)) _ns.push_back(node); }
        void remove() {
           printf (":::removing!\n");
           for (list<SgNode *>::iterator i = _ns.begin(); i != _ns.end(); i++) {
              SgNode *n = *i;
              printf(":::> %s\n", n->sage_class_name());
              delete n;
           }
        }      
           
   };
*/
  // DeleteAST deleteTree;
  // deleteTree.traverse(project, postorder);
  // deleteTree.remove();
  //delete project;  
}   


/*************************************************************************/

py::object create_rose_node(SgNode *node) { 
   if (!node) return py::object(); // None
   py::object obj = py::object(PySgNode()); 
   string entity; 

   // find entity corresponding to SG class name
   entity = EntityManager::get_sg_entity(node->class_name());
   if (entity.empty()) {
      // let us traverse all items, and find the entity that is closer.
      int best_rank = -1;
      for (auto e: EntityManager::get_entities()) {
          EntityInfo *einfo = EntityManager::expect_entity(e);
          if ((einfo->check_fn)(node)) {
              if (einfo->meta_rank > best_rank) {
                  best_rank = einfo->meta_rank;
                  entity = e;               
              }
          }
      }
      if (best_rank == -1) {
         throw rose_exception((string("internal error: cannot create Python ROSE node for SgNode: ") + node->class_name() + "!").c_str());   
      }
     
      // cache
      EntityManager::set_sg_entity(node->class_name(), entity);
   }
   

   EntityInfo *entity_info = EntityManager::get_entity(entity);
   hAssert(entity_info, "internal error: expecting entitty [%s] in EntityManager library!", entity.c_str());
   
   EntityInfo::CreateFn fn_create = entity_info->create_fn;

   //printf("::::> creating ROSE node: %s %p [%p]\n", node->class_name().c_str(), node, obj.ptr());
  
   (*fn_create)(node, obj);
   Py_INCREF(obj.ptr());
   // let's keep tab of all python ROSE nodes
   PrjAttribute *attr = (PrjAttribute *) node->getAttribute("project");
   //hAssert(attr, "Unable to create python ROSE node [%s]: it is not associated to project!", node->class_name().c_str());   
   
   if (attr) {    
      SgProject *prj = attr->prj();
      rose_objs[prj].push_back(obj); 
      hAssert(Py_REFCNT(obj.ptr()) == 3, "Generating Python ROSE object (%s) with reference count different than three: %d!", node->class_name().c_str(), Py_REFCNT(obj.ptr()));      
   } else {
      hAssert(Py_REFCNT(obj.ptr()) == 2, "Generating Python ROSE object (%s) with reference count different than two: %d!", node->class_name().c_str(), Py_REFCNT(obj.ptr()));      

   }
   
   return obj;
}

void PySgNode::del(py::object self) {
    SgNode *node = (SgNode *) to_sgnode(self);
    //printf(":::> removing ROSE node: %s %p [%p]\n", node->sage_class_name(), node, self.ptr());
    fflush(stdout);     
} 

string PySgNode::str(py::object self) {
    string entity = py::extract<string>(self.attr("entity"));
    string sg_type = py::extract<string>(self.attr("sg_type"));
    string sg_type_real = py::extract<string>(self.attr("sg_type_real"));
    string unparse = py::extract<string>(self.attr("unparse")());
        
    unparse = unparse.substr(0,100);
    if (unparse.back() != '\n') unparse += "\n";
    std::string  res = "\u001b[36m" + entity + "\u001b[31m" + " (" + 
    ((sg_type == sg_type_real)? sg_type : sg_type + ":" + sg_type_real)
    + ")" + "\u001b[0m" + 
            ": " + "\u001b[33m" + unparse + "\u001b[0m";    

    return res;
}

