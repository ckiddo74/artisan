#pragma once

#include "Expr.h"

NODE_SPEC_BEGIN(BinOp, SgBinaryOp, Expr, node, obj) {     
#if 0
    // BUGGY!
    if (isSgAddOp(node)) {
        obj.attr("tag") = "+";
    } else if (isSgDivideOp(node)) {
        obj.attr("tag") = "/";
    } else if (isSgMultiplyOp(node)) {
        obj.attr("tag") = "*";
    } else if (isSgSubtractOp(node)) {
        obj.attr("tag") = "-";
    } else if (isSgAssignOp(node)) {
        obj.attr("tag") = "=";
    }  
#endif
    bind_method(obj, "lhs", lhs);
    bind_method(obj, "rhs", rhs);    
    bind_method(obj, "op", op);
    bind_method(obj, "is_assign", is_assign);    
    bind_method(obj, "is_add", is_add);                  
    bind_method(obj, "is_mul", is_mul);    
    bind_method(obj, "is_sub", is_sub);    
    bind_method(obj, "is_div", is_div);    
}
   
static py::object lhs(py::object self) {  
    SgBinaryOp *node = (SgBinaryOp *) get_ir(self);
    py::object obj = create_rose_node(node->get_lhs_operand()); 
    return obj;
}   
    
static py::object rhs(py::object self) {
    SgBinaryOp *node = (SgBinaryOp *) get_ir(self);
    return create_rose_node(node->get_rhs_operand());
}  

static std::string op(py::object self) {
    auto node = (SgBinaryOp *) get_ir(self);
    switch (node->variantT()) {
        case V_SgAssignOp: return "=";
        case V_SgAddOp: return "+";
        case V_SgMultiplyOp: return "*";
        case V_SgSubtractOp: return "-";
        case V_SgDivideOp: return "/";        
        default: return "";
    }
}

static bool is_assign(py::object self) {
    auto node = (SgBinaryOp *) get_ir(self);
    return isSgAssignOp(node);
}
static bool is_add(py::object self) {
    auto node = (SgBinaryOp *) get_ir(self);
    return isSgAddOp(node);
}

static bool is_mul(py::object self) {
    auto node = (SgBinaryOp *) get_ir(self);
    return isSgMultiplyOp(node);
}

static bool is_sub(py::object self) {
    auto node = (SgBinaryOp *) get_ir(self);
    return isSgSubtractOp(node);
}

static bool is_div(py::object self) {
    auto node = (SgBinaryOp *) get_ir(self);
    return isSgDivideOp(node);
}



NODE_SPEC_END


