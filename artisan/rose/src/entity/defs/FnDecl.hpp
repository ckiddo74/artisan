#pragma once

#include "Decl.hpp"

ENTITY_SPEC_BEGIN(FnDecl, "Function Declaration", SgFunctionDeclaration, Decl, node, obj, entity, sg_type) {
    
    std::string name;
    
    if (node) {
        name = node->get_name().getString();
    }

    bind_attr(obj, "name", "function name", name);

    bind_method(obj, "tag", "function name", tag);   
    bind_method(obj, "params", "returns list of function parameters", params); 
    bind_method(obj, "return_type", "return type", return_type);    
}

static std::string tag(SgNodePtr self) {
     SgFunctionDeclaration *node = isSgFunctionDeclaration(self);

     return node->get_name().getString(); 

}

static py::list params(py::object self) {
    SgFunctionDeclaration *node = to_sgnode(self, SgFunctionDeclaration);   
    py::list lst = py::list();
    SgFunctionParameterList *param_list = node->get_parameterList ();
    hAssert(param_list, "internal error: FunctionParameterList of function [%s] is NULL!", node->get_name().getString());
    for (auto param: param_list->get_args()) {
        lst.append(create_rose_node(param));
    }

    return lst;
}

static py::object return_type(py::object self) {
    SgFunctionDeclaration *node = to_sgnode(self, SgFunctionDeclaration);   
    return create_rose_node(node->get_orig_return_type());

}

ENTITY_SPEC_END


