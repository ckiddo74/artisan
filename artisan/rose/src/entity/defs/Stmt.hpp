#pragma once

#include "SrcNode.hpp"

ENTITY_SPEC_BEGIN(Stmt, "code statements", SgStatement, SrcNode, node, obj, entity, sg_type) {
    bind_method(obj, "index", "returns child position in relation to its parent", index);   
}

static int index(py::object self) {
    auto stmt = to_sgnode(self, SgStatement);

    SgNode *parent = stmt->get_parent();

    hAssert(parent, "(internal error) parent is NULL!");
    
    return parent->get_childIndex(stmt);
}


   
ENTITY_SPEC_END

