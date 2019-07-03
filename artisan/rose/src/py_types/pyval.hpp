#pragma once

#include <boost/any.hpp>
#include <string>

class PyVal {

public:   
   enum Type { pv_err, pv_int, pv_real, pv_str };
   
protected:
   boost::any _val;
   Type _type;
   
public:
   PyVal() { _type = pv_err; }
   PyVal(int64_t val) { _type = pv_int; _val = val; }
   PyVal(int val) { _type = pv_int; _val = (int64_t) val; }   
   PyVal(double val) { _type = pv_real; _val = val; }
   PyVal(std::string val) { _type = pv_str; _val = val; }
   PyVal(const char *val) { _type = pv_str; _val = (std::string) val; }
   
   template <class T>
   operator T () { return boost::any_cast<T> (_val); }

   template <class T>
   operator T () const { return boost::any_cast<T> (_val); }
         
   Type type() const { return _type; }  
      
};
  
struct pyval_converter {
    static PyObject* convert(PyVal const& v)
      {
        PyVal::Type t = v.type();
        boost::python::object ret;
        switch (t) {
           case PyVal::pv_int:   ret = boost::python::object((int64_t) v); return boost::python::incref(ret.ptr()); break;
           case PyVal::pv_real:  ret = boost::python::object((double) v); return boost::python::incref(ret.ptr()); break;
           case PyVal::pv_str:   std::string str = v; ret = boost::python::object(str); return boost::python::incref(ret.ptr()); break;
        }   
        throw std::runtime_error("cannot translate value!"); 
     
     }   

  static void* convertible(PyObject* obj_ptr)
    {

      if (!PyLong_Check(obj_ptr) && !PyUnicode_Check(obj_ptr) && !PyFloat_Check(obj_ptr) ) return 0;
      return obj_ptr;
    }
    
  static void construct(
    PyObject* obj_ptr,
    boost::python::converter::rvalue_from_python_stage1_data* data)
    {
      PyVal value;
      
      if (PyLong_Check(obj_ptr)) { 
         value = (int64_t) PyLong_AsLong(obj_ptr);
      } else if (PyUnicode_Check(obj_ptr)) {
         value = (std::string) PyUnicode_AsUTF8(obj_ptr);         
      } else if (PyFloat_Check(obj_ptr)) {
         value = (double) PyFloat_AsDouble(obj_ptr);          
      }
      
      void* storage = (
        (boost::python::converter::rvalue_from_python_storage<PyVal>*)
        data)->storage.bytes;
 
      new (storage) PyVal(value);
 
      // Stash the memory chunk pointer for later use by boost.python
      data->convertible = storage;
    }     
     
     
     pyval_converter() {
        boost::python::to_python_converter<PyVal, pyval_converter> ();   
        boost::python::converter::registry::push_back(
           &convertible,
           &construct,
           boost::python::type_id<PyVal>()
        );
     }     
    

}; 

 

