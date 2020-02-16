#pragma once

#include "Scope.hpp"

ENTITY_SPEC_BEGIN(ForLoop, "FOR loop construct", SgForStatement, Scope, node, obj, entity, sg_type) { 
    bind_method(obj, "body", "loop body", body);          
}

static py::object body(py::object self) {
    SgForStatement *for_loop = to_sgnode(self, SgForStatement);
    return create_rose_node(for_loop->get_loop_body());
}   

ENTITY_SPEC_END




