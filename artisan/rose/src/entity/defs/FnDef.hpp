#pragma once

#include "Scope.hpp"

ENTITY_SPEC_BEGIN(FnDef, "function definition (function declaration + body)", SgFunctionDefinition, Scope, node, obj, entity, sg_type) {    
    bind_method(obj, "decl", "declaration", decl);      
    bind_method(obj, "body", "body", body);

    std::string name;

    if (node) {
        name = node->get_declaration()->get_name().getString(); 
    }
    
    bind_attr(obj, "name", "function name", name);
    bind_attr(obj, "tag", "function name", name);    
}

static py::object body(py::object self) {
    SgFunctionDefinition *node = to_sgnode(self, SgFunctionDefinition);   
    return create_rose_node(node->get_body());
}
static py::object decl(py::object self) {
    SgFunctionDefinition *node = to_sgnode(self, SgFunctionDefinition);   
    return create_rose_node(node->get_declaration());
}

ENTITY_SPEC_END



