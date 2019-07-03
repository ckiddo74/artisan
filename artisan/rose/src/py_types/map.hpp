#pragma once

#include <string>
#include <map>
#include <boost/python.hpp>

template<class T>
struct map_converter {
    static PyObject* convert(std::map<std::string,T> const& m)
      {
        boost::python::dict ret = boost::python::dict(); 
        for (typename std::map<std::string, T>::const_iterator i = m.begin(); i != m.end(); i++) {
           ret[i->first] = i->second;
        }
        return boost::python::incref(ret.ptr());
     }
    

  static void* convertible(PyObject* obj_ptr)
    {
      if (!PyDict_Check(obj_ptr)) return 0;
      return obj_ptr;
    }
    
  static void construct(
    PyObject* obj_ptr,
    boost::python::converter::rvalue_from_python_stage1_data* data)
    {
    
      boost::python::dict dict(boost::python::handle<>(boost::python::borrowed(obj_ptr)));
      std::map<std::string, T> value;

      boost::python::list keys = dict.keys();  
      for (boost::python::ssize_t i = 0; i < boost::python::len(keys); i++) {        
         std::string key = boost::python::extract<std::string>(keys[i]);
         value[key] = boost::python::extract<T>(dict[keys[i]]);
      }   
                
      void* storage = (
        (boost::python::converter::rvalue_from_python_storage<std::map<std::string, T> >*)
        data)->storage.bytes;
 
      new (storage) std::map<std::string, T> (value);
 
      // Stash the memory chunk pointer for later use by boost.python
      data->convertible = storage;
    }
   

    map_converter() {
      boost::python::to_python_converter<std::map<std::string, T>, map_converter<T> > ();
      boost::python::converter::registry::push_back(
        &convertible,
        &construct,
        boost::python::type_id<std::map<std::string, T> > ());    
  
    }    
        
}; 

