#pragma once

#include "Scope.hpp"

ENTITY_SPEC_BEGIN(DoLoop, "do while loop construct", SgDoWhileStmt, Scope, node, obj, entity, sg_type) { 
    bind_method(obj, "body", "loop body", body);        
}

static py::object body(py::object self) {
    SgDoWhileStmt *do_loop = (SgDoWhileStmt *) to_sgnode(self);
    return create_rose_node(do_loop->get_body());
}   

ENTITY_SPEC_END


