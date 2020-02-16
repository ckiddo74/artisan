#pragma once

#include "Scope.hpp"

ENTITY_SPEC_BEGIN(WhileLoop, "while loop construct", SgWhileStmt, Scope, node, obj, entity, sg_type) { 
    bind_method(obj, "body", "loop body", body);       
}

static py::object body(py::object self) {
    SgWhileStmt *while_stmt = (SgWhileStmt *) to_sgnode(self);
    return create_rose_node(while_stmt->get_body());
}   

ENTITY_SPEC_END



