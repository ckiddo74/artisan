#pragma once

#include <boost/python.hpp>

boost::python::object __query(boost::python::object root, std::string match, std::string condition, 
                              boost::python::object env);
                                 
                                