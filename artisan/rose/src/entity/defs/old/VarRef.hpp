#pragma once

#include "Expr.h"

NODE_SPEC_BEGIN(VarRef, SgVarRefExp, Expr, node, obj) {
    std::string name = node->get_symbol()->get_name().getString();
    obj.attr("tag") = name;
    obj.attr("name") = name;                  
}
NODE_SPEC_END




