#pragma once

#include "Scope.hpp"

ENTITY_SPEC_BEGIN(Global, "AST node representing global scope", SgGlobal, Scope, node, obj, entity, sg_type) {
    bind_method(obj, "stmts", "get global statements", stmts);   
}

static py::list stmts(py::object self) {
    auto global = to_sgnode(self, SgGlobal);
    std::vector<SgDeclarationStatement *> statements = global->get_declarations();
    return create_rose_nodes(statements.begin(), statements.end());
}

ENTITY_SPEC_END


