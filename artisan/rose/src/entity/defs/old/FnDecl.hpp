#pragma once

#include "Decl.h"

NODE_SPEC_BEGIN(FnDecl, SgFunctionDeclaration, Decl, node, obj) {
    std::string name = node->get_name().getString();

    obj.attr("name") =  name;
    obj.attr("tag") = name;  
    
    bind_method(obj, "params", params); 
    bind_method(obj, "return_type", return_type);    
}

static py::list params(py::object self) {
    SgFunctionDeclaration *node = (SgFunctionDeclaration *) get_ir(self);   
    py::list lst = py::list();
    SgFunctionParameterList *param_list = node->get_parameterList ();
    hAssert(param_list, "internal error: FunctionParameterList of function [%s] is NULL!", node->get_name().getString());
    for (auto param: param_list->get_args()) {
        lst.append(create_rose_node(param));
    }

    return lst;
}

static py::object return_type(py::object self) {
    SgFunctionDeclaration *node = (SgFunctionDeclaration *) get_ir(self);   
    return create_rose_node(node->get_orig_return_type());

}

NODE_SPEC_END


