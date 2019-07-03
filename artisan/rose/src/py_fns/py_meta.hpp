#pragma once

#include <boost/python.hpp>

boost::python::list __meta_entities(std::string match="");

BOOST_PYTHON_FUNCTION_OVERLOADS(meta_entities_overload, __meta_entities, 0, 1)
                                 

boost::python::dict __meta_info(std::string entity);
