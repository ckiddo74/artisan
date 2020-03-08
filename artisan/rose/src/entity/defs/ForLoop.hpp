#pragma once

#include "Scope.hpp"
#include <entity/Loop.hpp>

ENTITY_SPEC_BEGIN(ForLoop, "FOR loop construct", SgForStatement, Scope, node, obj, entity, sg_type) { 
    bind_method(obj, "body", "loop body", body);    
    bind_method(obj, "init", "init [statements]: for (init;..;..) { }", for_init);
    bind_method(obj, "cond", "cond statement: for (...; cond ;..) { }", for_cond);
    bind_method(obj, "incr", "increment expression: for (...; ... ; incr) { }", for_expr);
    bind_method(obj, "tag", "loop id", tag);  
    bind_method(obj, "is_outermost", "returns True if loop is the top-level loop", is_outermost, (ARG("for_loop", true), ARG("while_loop", false), ARG("do_loop", false)));
    bind_method(obj, "is_innermost", "returns True if loop is innermost", is_innermost, (ARG("for_loop", true), ARG("while_loop", false), ARG("do_loop", false)));
}


static std::string tag(SgNodePtr self) {
     SgForStatement *node = isSgForStatement(self);

     return LoopUtils::loop_tag(node, "for", V_SgForStatement);
}

static py::object body(py::object self) {
    SgForStatement *for_loop = to_sgnode(self, SgForStatement);
    return create_rose_node(for_loop->get_loop_body());
}   

static py::list for_init(py::object self) {
    SgForStatement *for_loop = to_sgnode(self, SgForStatement);
    py::list lst = py::list();    

    SgForInitStatement *init_stmt = for_loop->get_for_init_stmt();
    if (init_stmt) {
        const SgStatementPtrList &stmt_lst = init_stmt->get_init_stmt();
        for (auto stmt : stmt_lst) {
            lst.append(create_rose_node(stmt));
        }
    }

    return lst;
}

static py::object for_cond(py::object self) {
    SgForStatement *for_loop = to_sgnode(self, SgForStatement);
    return create_rose_node(for_loop->get_test());
}  

static py::object for_expr(py::object self) {
    SgForStatement *for_loop = to_sgnode(self, SgForStatement);
    return create_rose_node(for_loop->get_increment());
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




