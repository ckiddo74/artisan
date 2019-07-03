#pragma once

#include <rose.h>
#include <list>
#include <map>
#include <boost/python.hpp>
#include <boost/regex.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/preprocessor/facilities/overload.hpp>
#include <utils/hmsg.hpp>
#include <py_types/sgnode_ptr.hpp>
#include <py_sgnode.hpp>

struct EntityDoc {      
    bool is_method; // false = attribute, true = method
    std::string desc;
    std::list<std::pair<std::string, std::string>> params;      
};
    
struct EntityInfo {
    typedef bool (*CheckFn)(SgNode *);
    typedef void (*CreateFn)(SgNode *, boost::python::object);
    typedef void (*MetaFn) (EntityInfo *info);

    CheckFn check_fn;
    CreateFn create_fn;
    MetaFn meta_fn;
    enum VariantT sg_variant;

    // computed during meta
    int meta_rank;
    std::list<std::string> meta_entities;
    std::map<std:: string, EntityDoc> meta_doc;

    std::string description;

    EntityInfo(CheckFn chfn, CreateFn crfn, MetaFn mtfn, enum VariantT vt);

    void compute_meta();
};

#define __ARG_1(name)   boost::python::arg(name)
#define __ARG_2(name, val)   boost::python::arg(name)=val
#define ARG(...) BOOST_PP_OVERLOAD(__ARG_,__VA_ARGS__)(__VA_ARGS__)
#define __STR_ARGS(x) #x
#define bind_method(obj, name, description,  method, ...) \
    {\
        if (obj != boost::python::object()) { \
            boost::python::object f = boost::python::make_function(method, boost::python::default_call_policies(), ##__VA_ARGS__); \
            bool has_attr = PyObject_HasAttrString(obj.ptr(), name); \
            obj.attr(name) = f.attr("__get__")(obj); \
            if (!has_attr) Py_DECREF(obj.ptr());\
        } else {  \
            if (description != "") { \
               EntityDoc &doc = (*__DOC__)[name]; \
               doc.is_method = true; \
               doc.desc = description; \
               doc.params = Entity::convert_params(#__VA_ARGS__); \
            } \
        } \
    } 

#define bind_attr(obj, name, description, val) \
    if (obj != boost::python::object ()) { \
        obj.attr(name) = val; \
    } else {  \
        if (description != "") { \
            EntityDoc &doc = (*__DOC__)[name]; \
            doc.is_method = false; \
            doc.desc = description; \
        } \
    }   

#define ENTITY_SPEC_BEGIN(cls, entity_desc, sg_type_name, parent, __node__, __obj__, __entity__, __sg_type__) \
\
namespace entity { \
\
namespace py = boost::python; \
using namespace std; \
\
class cls: public parent { \
public: \
   static constexpr const char *_entity = #cls;\
   static constexpr const char *_sg_type     = #sg_type_name;\
   static constexpr const int _sg_variant = sg_type_name::static_variant; \
   static void create(SgNode *node, py::object obj) { \
      cls x(node, obj, _entity, _sg_type); \
   } \
   static bool check(SgNode *node) { \
      return is##sg_type_name (node);  \
   }   \
   static void meta(EntityInfo *info) { \
      cls x; \
      x.init(0, py::object(), "", "", &info->meta_doc); \
      info->description = entity_desc; \
      info->meta_entities.clear(); \
      cls::meta_entities(info->meta_entities); \
      info->meta_rank = (int) info->meta_entities.size(); \
   } \
   static void meta_entities(std::list<std::string> &entities) { \
      entities.push_back(_entity); \
      parent::meta_entities(entities); \
   } \
   cls(SgNode *node, py::object obj, std::string entity, std::string sg_type): parent(node, obj, entity, sg_type) { \
      init((sg_type_name *) node, obj, entity, sg_type, 0); \
   } \
   cls() { } \
   void init(sg_type_name * __node__, py::object __obj__, std::string __entity__, std::string __sg_type__,  std::map<std:: string, EntityDoc> *__DOC__)   

#define ENTITY_SPEC_END }; }
 
namespace entity {   

namespace py = boost::python;

class Entity { 
public:     
   Entity(SgNode *node, py::object obj, std::string entity, std::string sg_type) {  }   
   Entity() { }    
   static void meta_entities(std::list<std::string> &entities) {  }
   static std::list<std::pair<std::string, std::string> > convert_params(std::string params);
};

}

