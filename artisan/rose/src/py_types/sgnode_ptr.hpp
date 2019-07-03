#pragma once

#include <py_sgnode.hpp>

class SgNodePtr {
public:    
   SgNodePtr(SgNode *node) {
       _node = node;
   }

   operator SgNode* () const { return _node; }

protected:
    SgNode *_node;
};

struct sgnode_ptr_converter {
     
    static PyObject* convert(SgNodePtr const& v)
      {
        boost::python::object obj = create_rose_node(v);
        Py_INCREF(obj.ptr());
        return obj.ptr();
      }        

    static void* convertible(PyObject *obj) {
        return obj;
    }

    static void construct(
        PyObject* obj_ptr,
        boost::python::converter::rvalue_from_python_stage1_data* data)
    {
        // Extract the character data from the python string
        boost::python::object obj(boost::python::handle<>(boost::python::borrowed(obj_ptr)));
        
        SgNode *value = to_sgnode(obj);

        void* storage = (
          (boost::python::converter::rvalue_from_python_storage<SgNodePtr>*)
          data)->storage.bytes;

        new (storage) SgNodePtr(value);

        data->convertible = storage;
    }    

     sgnode_ptr_converter() {
        boost::python::to_python_converter<SgNodePtr, sgnode_ptr_converter> ();   
        boost::python::converter::registry::push_back(
           &convertible,
           &construct,
           boost::python::type_id<SgNodePtr>()
        );
     }   

};