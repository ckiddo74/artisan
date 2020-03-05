#pragma once

#include "Expr.hpp"

ENTITY_SPEC_BEGIN(ExprDelete, "Represents delete operator", SgDeleteExp, Expr, node, obj, entity, sg_type) {
    bind_method(obj, "var", "returns variable to be freed", var);      
}

static py::object var(py::object self) {
    SgDeleteExp *del_expr = to_sgnode(self, SgDeleteExp);
    return create_rose_node(del_expr->get_variable ());
}  

ENTITY_SPEC_END



