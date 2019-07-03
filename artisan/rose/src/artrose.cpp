#include <rose.h>

#include <csignal>
#include <exception>
#include <boost/python.hpp>
#include <boost/algorithm/string.hpp>

// type conversions
#include <py_types/py_types.hpp>

#include <utils/hmsg.hpp>

#include <entity/EntityManager.hpp>
#include <py_sgnode.hpp>
#include <py_fns/py_query.hpp>
#include <py_fns/py_unparse_prj.hpp>
#include <py_fns/py_meta.hpp>

using namespace std;

namespace py = boost::python;

int hVerboseLevel::_verbose_level = 0;
string hVerboseLevel::_verbose_tag = "";

void seg_fault(int sig) {
   throw rose_exception("signal crash detected!"); 
}

void translate(exception const& e)
{
    // Use the Python 'C' API to set up an exception object
    PyErr_SetString(PyExc_RuntimeError, e.what());
}

void __set_verbose_level(py::tuple lvl) {
    
   int nlevel = py::extract<int>(lvl[0]);
   string tag = py::extract<string>(lvl[1]);

   hVerboseLevel::set(nlevel, tag);
   
   if (nlevel > 2 && (tag == "" || boost::algorithm::starts_with(tag , "rose"))) {
      SgProject::set_verbose(nlevel - 2);
   } else {
      SgProject::set_verbose(0);
   } 
} 

BOOST_PYTHON_MODULE(artrose)
{

    py::register_exception_translator<std::exception>(&translate);
        
    signal(SIGSEGV, seg_fault);   
    
    py::def("frontend", __frontend);
    py::def("destroy_project", __destroy_project);

    py::def("set_verbose_level", __set_verbose_level);
    py::def("unparse_prj", __unparse_prj);
    py::def("meta_entities", __meta_entities, meta_entities_overload());
    py::def("meta_info", __meta_info);    
    
    try {
       EntityManager::register_entities();  

    } catch (exception &e) {
        printf("=============\n%s\n==================================================\n", e.what());
        throw e;
    }
    
    py::class_<PySgNode>("PySgNode")
       .def("__del__", PySgNode::del) 
       .def("__repr__", PySgNode::str);
    
    sgnode_ptr_converter();

    list_converter<string>();
    list_converter<int>();    
    list_converter<map<string, string> >();
    list_converter<map<string, int> >();
    list_converter<SgNodePtr>();   
    map_converter<string>();
    map_converter<int>();

    

#if 0
    pyval_converter();
    list_converter<PyVal>();
    map_converter<PyVal>();        
    tuple_converter<PyVal, PyVal>();
    tuple_converter<PyVal, PyVal, PyVal>();    
    tuple_converter<PyVal, PyVal, PyVal, PyVal>();    
    tuple_converter<PyVal, PyVal, PyVal, PyVal, PyVal>();    
    tuple_converter<PyVal, PyVal, PyVal, PyVal, PyVal, PyVal>();  
    tuple_converter<PyVal, PyVal, PyVal, PyVal, PyVal, PyVal, PyVal>();  
    tuple_converter<PyVal, PyVal, PyVal, PyVal, PyVal, PyVal, PyVal, PyVal>();            
#endif
}

