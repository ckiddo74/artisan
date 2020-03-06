#pragma once

#include "NodeSrc.hpp"

ENTITY_SPEC_BEGIN(Expr, "expression", SgExpression, NodeSrc, node, obj, entity, sg_type) {
    bind_method(obj, "type", "returns expression type", type);
    bind_method(obj, "is_val", "returns whether expression represents a constant value", is_val);
    bind_method(obj, "stmt", "returns (parent) statement associated with this expression", stmt);
}  

static py::object type(py::object self) {
    SgExpression *node = (SgExpression *) to_sgnode(self);   
    return create_rose_node(node->get_type());
}

static bool is_val(py::object self) {
    SgExpression *node = (SgExpression *) to_sgnode(self);   
    return (bool) isSgValueExp(node);
}

static py::object stmt(py::object self) {
    SgExpression *node = (SgExpression *) to_sgnode(self);   
    SgNode *parent = node->get_parent();
    while (parent && !isSgStatement(parent)) {
        parent = parent->get_parent();
    }
    return create_rose_node(parent);    
}
  
ENTITY_SPEC_END

