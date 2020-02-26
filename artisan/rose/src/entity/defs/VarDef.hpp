#pragma once

#include "SrcNode.hpp"

ENTITY_SPEC_BEGIN(VarDef, "Variable Definition", SgInitializedName, SrcNode, node, obj, entity, sg_type) {
    std::string name;

    if (node) {
       name = node->get_name().getString();
    }

    bind_attr(obj, "name", "variable name", name);
    bind_method(obj, "type", "variable type", type); 
    bind_method(obj, "initializer", "variable initialization", initializer);
    bind_method(obj, "decl", "variable declaration statement", decl);    
}

static std::string tag(SgNodePtr self) {
     SgInitializedName *node = isSgInitializedName(self);

     return node->get_name().getString(); 

}

static py::object type(py::object self) {
    SgInitializedName *node = to_sgnode(self, SgInitializedName);  
    return create_rose_node(node->get_type());
}

static py::object initializer(py::object self) {
    SgInitializedName *node = to_sgnode(self, SgInitializedName);
    return create_rose_node(node->get_initializer());
}

static py::object decl(py::object self) {
    SgInitializedName *node = to_sgnode(self, SgInitializedName);  
    return create_rose_node(node->get_declaration());    
}


ENTITY_SPEC_END
