#pragma once

#include "Expr.h"

NODE_SPEC_BEGIN(UniOp, SgUnaryOp, Expr, node, obj)  {
#if 0 
    // BUGGY inside query   
    if (isSgPlusPlusOp(node)) {
        obj.attr("tag") = "++";
    } else if (isSgMinusMinusOp(node)) {
        obj.attr("tag") = "--";
    }
#endif                      
}
NODE_SPEC_END


