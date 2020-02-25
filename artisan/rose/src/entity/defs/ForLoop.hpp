#pragma once

#include "Scope.hpp"
#include <entity/Loop.hpp>

ENTITY_SPEC_BEGIN(ForLoop, "FOR loop construct", SgForStatement, Scope, node, obj, entity, sg_type) { 
    bind_method(obj, "body", "loop body", body);    
    bind_method(obj, "tag", "loop id", tag);              
}


static std::string tag(SgNodePtr self) {
     SgForStatement *node = isSgForStatement(self);

     return loop_tag(node, "for", V_SgForStatement);
}

static py::object body(py::object self) {
    SgForStatement *for_loop = to_sgnode(self, SgForStatement);
    return create_rose_node(for_loop->get_loop_body());
}   

ENTITY_SPEC_END




