#pragma once

#include <rose.h>
#include <string>
#include <list>
#include <boost/python.hpp>
#include <py_sgnode.hpp>
#include <boost/preprocessor/facilities/overload.hpp>


boost::python::object create_rose_node(SgNode *node);

template <typename Iter>
boost::python::list create_rose_nodes(Iter it, Iter end) {
    boost::python::list lst = boost::python::list();
    for (; it != end; ++it) {
        lst.append(create_rose_node(*it));
    }
    
    return lst;   

}

boost::python::object __frontend(std::list<std::string> args);
void __destroy_project(boost::python::object prj);

class PrjAttribute: public AstAttribute {
   SgProject *_project;

public:
   PrjAttribute(SgProject *project) { _project = project; }
   SgProject *prj() { return _project; }

   virtual AstAttribute::OwnershipPolicy getOwnershipPolicy() const {
      return AstAttribute::CUSTOM_OWNERSHIP;
   }  
};
      
struct PySgNode { 
   static void del(boost::python::object self);
   static std::string str(boost::python::object self);
};

#define __to_sgnode_2(obj, Type) (Type *) (int64_t) boost::python::extract<int64_t>(obj.attr("uid"))
#define __to_sgnode_1(obj) __to_sgnode_2(obj, SgNode)
#define to_sgnode(...) BOOST_PP_OVERLOAD(__to_sgnode_,__VA_ARGS__)(__VA_ARGS__)

#define assert_entity(obj, entity) \
   hEXCEPTION_IF(((std::string) boost::python::extract<std::string>(obj.attr("entity"))) != entity, "invalid object - expecting '%s' type object!", entity)
      

