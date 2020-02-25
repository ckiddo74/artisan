#pragma once

#include "Scope.hpp"
#include <entity/Loop.hpp>

ENTITY_SPEC_BEGIN(DoLoop, "do while loop construct", SgDoWhileStmt, Scope, node, obj, entity, sg_type) { 
    bind_method(obj, "body", "loop body", body);        
    bind_method(obj, "tag", "loop id", tag);       
}

static py::object body(py::object self) {
    SgDoWhileStmt *do_loop = (SgDoWhileStmt *) to_sgnode(self);
    return create_rose_node(do_loop->get_body());
}   

static std::string tag(SgNodePtr self) {
     SgDoWhileStmt *node = isSgDoWhileStmt(self);

     return loop_tag(node, "do", V_SgDoWhileStmt);
}


ENTITY_SPEC_END


