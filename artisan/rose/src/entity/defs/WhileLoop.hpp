#pragma once

#include "Scope.hpp"
#include <entity/Loop.hpp>

ENTITY_SPEC_BEGIN(WhileLoop, "while loop construct", SgWhileStmt, Scope, node, obj, entity, sg_type) { 
    bind_method(obj, "body", "loop body", body);  
    bind_method(obj, "tag", "loop id", tag);      
}

static py::object body(py::object self) {
    SgWhileStmt *while_stmt = (SgWhileStmt *) to_sgnode(self);
    return create_rose_node(while_stmt->get_body());
}   

static std::string tag(SgNodePtr self) {
     SgWhileStmt *node = isSgWhileStmt(self);

     return loop_tag(node, "while", V_SgWhileStmt);
}


ENTITY_SPEC_END



