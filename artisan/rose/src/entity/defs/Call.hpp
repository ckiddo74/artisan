#pragma once

#include "Expr.hpp"

ENTITY_SPEC_BEGIN(Call, "Call expression", SgCallExpression, Expr, node, obj, entity, sg_type) {
    bind_method(obj, "stmt", "returns parent statement", stmt);          
    bind_method(obj, "name", "returns function name (if it can be determined statically, '' otherwise)", name); 
}

static py::object stmt(py::object self) {
    SgCallExpression *call = to_sgnode(self, SgCallExpression);
    SgNode *node = call->get_parent();
    while (!isSgStatement(node)) {
        node = node->get_parent();
    }
    return create_rose_node(node);
}   

static std::string name(py::object self) {
    SgFunctionCallExp *call = isSgFunctionCallExp(to_sgnode(self, SgCallExpression));
    if (call) {
        SgFunctionSymbol *sym = call->getAssociatedFunctionSymbol();
        return sym->get_name().getString();
    } else {
        return "";
    }
       

}

ENTITY_SPEC_END



