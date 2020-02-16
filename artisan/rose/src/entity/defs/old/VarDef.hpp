#pragma once

#include "SrcNode.h"

NODE_SPEC_BEGIN(VarDef, SgInitializedName, SrcNode, node, obj) {
    std::string name = node->get_name().getString();
    obj.attr("name") =  name;
    obj.attr("tag") = name;  

    bind_method(obj, "type", type); 
    bind_method(obj, "initializer", initializer);
    bind_method(obj, "decl", decl);    
}

static py::object type(py::object self) {
    SgInitializedName *node = (SgInitializedName *) get_ir(self);  
    return create_rose_node(node->get_type());
}

static py::object initializer(py::object self) {
    SgInitializedName *node = (SgInitializedName *) get_ir(self);  
    return create_rose_node(node->get_initializer());
}

static py::object decl(py::object self) {
    SgInitializedName *node = (SgInitializedName *) get_ir(self);  
    return create_rose_node(node->get_declaration());    
}


NODE_SPEC_END
