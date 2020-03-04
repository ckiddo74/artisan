#pragma once

#include "Decl.hpp"

ENTITY_SPEC_BEGIN(Pragma, "Declaration statement", SgPragmaDeclaration, Stmt, node, obj, entity, sg_type) {
    bind_method(obj, "directive", "returns directive associated with the pragma", directive);
}

static std::string directive(SgNodePtr self) {
     SgPragmaDeclaration *node = isSgPragmaDeclaration(self);

     return node->get_pragma()->get_pragma();
}     

ENTITY_SPEC_END





