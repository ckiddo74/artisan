#pragma once

#include "Node.hpp"

ENTITY_SPEC_BEGIN(Project, "application code (root AST node)", SgProject, Node, node, obj, entity, sg_type) {
    bind_method(obj, "children", "my children - project", children);
    bind_method(obj, "num_children", "", num_children);   
}
   
static py::object children(py::object obj) {
    SgProject *node = (SgProject *) to_sgnode(obj);  
    py::list lst = py::list();
    size_t n = node->numberOfFiles();
    for (size_t i = 0; i < n; i++) {
        SgNode *child = (*node)[i]; 
        lst.append(create_rose_node(child));
    }
    
    return lst;
}   

static int num_children(py::object obj) {
    SgProject *node = (SgProject *) to_sgnode(obj);  
    return node->numberOfFiles();
} 

ENTITY_SPEC_END


