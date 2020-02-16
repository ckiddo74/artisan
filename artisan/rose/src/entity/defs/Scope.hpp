#pragma once

#include "Stmt.hpp"

ENTITY_SPEC_BEGIN(Scope, "statements that have an associated symbol table (scope)", SgScopeStatement, Stmt, node, obj, entity, sg_type) {
    bind_method(obj, "stmts", "list of statements", stmts);   
    bind_method(obj, "stmts_count", "number of top-level statements", stmts_count);  
    
}

static py::list stmts(py::object self) {
    auto scope = to_sgnode(self, SgScopeStatement);
    std::vector<SgStatement *> statements = scope->getStatementList();
    return create_rose_nodes(statements.begin(), statements.end());
}

static int stmts_count(py::object self) {
    auto scope = to_sgnode(self, SgScopeStatement);

    return (int) scope->get_numberOfTraversalSuccessors();
}





ENTITY_SPEC_END



