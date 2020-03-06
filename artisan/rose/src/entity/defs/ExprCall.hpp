#pragma once

#include "Expr.hpp"

ENTITY_SPEC_BEGIN(ExprCall, "Call expression", SgCallExpression, Expr, node, obj, entity, sg_type) {
    bind_method(obj, "name", "returns function name (if function pointer, returns \"\")", name); 
}

static std::string name(py::object self) {
    SgFunctionCallExp *call = isSgFunctionCallExp(to_sgnode(self, SgCallExpression));
    if (call) {
        SgFunctionSymbol *sym = call->getAssociatedFunctionSymbol();
        if (sym) {
            return sym->get_name().getString();
        }
    } 
    return "";
       

}

ENTITY_SPEC_END



