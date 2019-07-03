#pragma once

#include <list>
#include <boost/python.hpp>

template<class T>
struct vector_converter {
   static PyObject* convert(std::vector<T> const& l) {
        boost::python::list ret = boost::python::list(); 
        for (typename std::vector<T>::const_iterator i = l.begin(); i != l.end(); i++) {
           ret.append(*i);
        }
        return boost::python::incref(ret.ptr());
   }
                 

  static void* convertible(PyObject* obj_ptr)
    {

      if (!PyList_Check(obj_ptr)) return 0;
      return obj_ptr;
    }
    
  static void construct(
    PyObject* obj_ptr,
    boost::python::converter::rvalue_from_python_stage1_data* data)
    {
    
      boost::python::list lst(boost::python::handle<>(boost::python::borrowed(obj_ptr)));
      std::vector<T> value; 
      for (boost::python::ssize_t i = 0; i < boost::python::len(lst); i++) {
         value.push_back(boost::python::extract<T>(lst[i]));   
      }
      
      void* storage = (
        (boost::python::converter::rvalue_from_python_storage<std::vector<T> >*)
        data)->storage.bytes;
 
      new (storage) std::vector<T> (value);
 
      // Stash the memory chunk pointer for later use by boost.python
      data->convertible = storage;
    }
   
       
    vector_converter() {
        boost::python::converter::registry::push_back(
           &convertible,
           &construct,
           boost::python::type_id<std::vector<T> > ());    

        boost::python::to_python_converter<std::vector<T>, vector_converter<T> > (); 
    }    
        
}; 
