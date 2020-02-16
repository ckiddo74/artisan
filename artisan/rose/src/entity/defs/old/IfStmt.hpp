#pragma once

#include "Scope.h"

NODE_SPEC_BEGIN(IfStmt, SgIfStmt, Scope, node, obj) {
    bind_method(obj, "cond", cond)
    bind_method(obj, "body", body);     
    bind_method(obj, "body_else", body_else);
}

static py::object cond(py::object self) {
    SgIfStmt *if_stmt = (SgIfStmt *) get_ir(self);
    return create_rose_node(if_stmt->get_conditional());
}

static py::object body(py::object self) {
    SgIfStmt *if_stmt = (SgIfStmt *) get_ir(self);
    return create_rose_node(if_stmt->get_true_body());
}   

static py::object body_else(py::object self) {
    SgIfStmt *if_stmt = (SgIfStmt *) get_ir(self);
    return create_rose_node(if_stmt->get_false_body());
}   

NODE_SPEC_END



