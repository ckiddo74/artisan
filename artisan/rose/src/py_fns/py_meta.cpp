#include <entity/EntityManager.hpp>
#include "py_meta.hpp"
#include <list>
#include <string>
#include <regex>
#include <utils/hmsg.hpp>

using namespace std;
namespace py = boost::python;


py::list __meta_entities(string match) {
    py::list lst;
    
    if (match == "") {    
        lst = py::list(EntityManager::get_entities());
    } else {
        regex r(match);
        for (auto e : EntityManager::get_entities()) {
            if (regex_match(e, r)) {
               lst.append(e);
            }
        }
    }
    
    return lst;
} 

py::dict __meta_info(string entity) {
    py::dict dict = py::dict();

    EntityInfo *info = EntityManager::expect_entity(entity);  

/*
    std::map<std:: string, EntityDoc> meta_doc;
*/  
    dict["name"] = entity;
    dict["description"] = info->description;
    dict["rank"] = info->meta_rank;
    dict["entities"] = py::list(info->meta_entities);
    py::dict doc = py::dict();
    dict["attributes"] = doc;
    
    for (auto &doc_entry: info->meta_doc) {
        py::list l_params = py::list();
        
        const list<pair<string, string>> &params = doc_entry.second.params;
        for (auto param: params) {
            hlog(3, "meta", "entity: %s, method: %s, param: %s%s", entity, doc_entry.first, param.first, param.second.empty()? string(): string("=") + param.second);
            if (param.second.empty()) {
               l_params.append(param.first);
            } else {
               l_params.append(py::make_tuple(param.first, py::eval(param.second.c_str())));                
            }
        } 
        doc[doc_entry.first] = py::dict();
        doc[doc_entry.first]["is_method"] = doc_entry.second.is_method;
        doc[doc_entry.first]["params"] = l_params;
        doc[doc_entry.first]["description"] = doc_entry.second.desc;
    }

    auto log_fn = [] (void *data) -> string {
        py::dict dict = py::dict((boost::python::handle<>(boost::python::borrowed((PyObject *) data))));
        string dict_str =  py::extract<string>(dict.attr("__repr__")());
        return dict_str;
    };  
    hlog_fn(3, "meta", log_fn, (void *) dict.ptr());    

    return dict;
}
