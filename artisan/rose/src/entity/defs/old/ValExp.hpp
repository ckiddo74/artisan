#pragma once

#include "Expr.h"

NODE_SPEC_BEGIN(ValExp, SgValueExp, Expr, node, obj) {
    std::string value = node->unparseToString();
    obj.attr("tag") = value;
    obj.attr("value") = value;   
                   
}
NODE_SPEC_END




