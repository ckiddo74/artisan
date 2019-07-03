#pragma once

#include <boost/python.hpp>
#include <string>


extern void py_instrument(boost::python::object self, std::string where, std::string code,  
                          boost::python::object env, bool auto_sp, bool append);
