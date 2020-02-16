#pragma once

#include "BinOp.h"

NODE_SPEC_BEGIN(ArrayRef, SgPntrArrRefExp, BinOp, node, obj)
{
      bind_method(obj, "var", var);
      bind_method(obj, "dims", dims);    
      bind_method(obj, "indices", indices);            
}
  
static std::string var (py::object self) {
    SgPntrArrRefExp *arr = (SgPntrArrRefExp *) get_ir(self);
    SgInitializedName *decl = SageInterface::convertRefToInitializedName(arr);
    return decl->get_name().getString();      
}

static py::object dims(py::object self) {
    py::list lst = py::list();
    
    SgPntrArrRefExp *arr = (SgPntrArrRefExp *) get_ir(self);
    SgInitializedName *decl = SageInterface::convertRefToInitializedName(arr);
    SgArrayType *type = isSgArrayType(decl->get_type());

    while (type) {
        lst.append(create_rose_node(type->get_index()));
        type = isSgArrayType(type->get_base_type());
    }      
    
    return lst;  

}

static py::object indices(py::object self) {
    py::list lst = py::list();
    
    SgPntrArrRefExp *arr = (SgPntrArrRefExp *) get_ir(self);
    
    std::list<SgExpression *> index_lst;

    while (arr) {
        index_lst.push_front(arr->get_rhs_operand()); 
        arr = isSgPntrArrRefExp(arr->get_lhs_operand());
    }      
    
    for (auto i: index_lst) {
        lst.append(create_rose_node(i));
    }
    
    return lst;  

}   

NODE_SPEC_END

