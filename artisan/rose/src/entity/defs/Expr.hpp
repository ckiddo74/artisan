#pragma once

#include "SrcNode.hpp"

ENTITY_SPEC_BEGIN(Expr, "expression", SgExpression, SrcNode, node, obj, entity, sg_type) {
    bind_method(obj, "type", "returns expression type", type);
    bind_method(obj, "is_val", "returns whether expression represents a constant value", is_val);


}  

static py::object type(py::object self) {
    SgExpression *node = (SgExpression *) to_sgnode(self);   
    return create_rose_node(node->get_type());
}

static bool is_val(py::object self) {
    SgExpression *node = (SgExpression *) to_sgnode(self);   
    return (bool) isSgValueExp(node);
}
  
ENTITY_SPEC_END

