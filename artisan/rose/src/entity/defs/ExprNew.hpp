#pragma once

#include "Expr.hpp"

ENTITY_SPEC_BEGIN(ExprNew, "Represents new operator", SgNewExp, Expr, node, obj, entity, sg_type) {
    bind_method(obj, "type", "returns type allocated", type);          
}

static py::object type(py::object self) {
    SgNewExp *new_expr = to_sgnode(self, SgNewExp);
    return create_rose_node(new_expr->get_type());
}   

ENTITY_SPEC_END



