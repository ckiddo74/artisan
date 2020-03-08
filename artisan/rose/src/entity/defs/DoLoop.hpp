#pragma once

#include "Scope.hpp"
#include <entity/Loop.hpp>

ENTITY_SPEC_BEGIN(DoLoop, "do while loop construct", SgDoWhileStmt, Scope, node, obj, entity, sg_type) { 
    bind_method(obj, "body", "loop body", body);        
    bind_method(obj, "tag", "loop id", tag);   
    bind_method(obj, "is_outermost", "returns True if loop is the top-level loop", is_outermost, (ARG("for_loop", false), ARG("while_loop", false), ARG("do_loop", true)));
    bind_method(obj, "is_innermost", "returns True if loop is innermost", is_innermost, (ARG("for_loop", false), ARG("while_loop", false), ARG("do_loop", true)));    

}

static py::object body(py::object self) {
    SgDoWhileStmt *do_loop = (SgDoWhileStmt *) to_sgnode(self);
    return create_rose_node(do_loop->get_body());
}   

static std::string tag(SgNodePtr self) {
     SgDoWhileStmt *node = isSgDoWhileStmt(self);

     return LoopUtils::loop_tag(node, "do", V_SgDoWhileStmt);
}

static bool is_outermost(py::object self, bool for_loop, bool while_loop, bool do_loop) {
    SgForStatement *node = to_sgnode(self, SgForStatement);    
    return LoopUtils::is_outermost(node, for_loop, while_loop, do_loop);
}

static bool is_innermost(py::object self, bool for_loop, bool while_loop, bool do_loop) {
    SgForStatement *node = to_sgnode(self, SgForStatement);    
    return LoopUtils::is_innermost(node, for_loop, while_loop, do_loop);
}

ENTITY_SPEC_END


