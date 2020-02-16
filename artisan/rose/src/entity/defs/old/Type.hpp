#pragma once

#include "Node.h"
#include <instrumentation/py_instrument.h>

NODE_SPEC_BEGIN(Type, SgType, Node, node, obj) {
    obj.attr("tag") = node->unparseToString();
    obj.attr("name") = node->unparseToString();
    bind_method(obj, "instrument", instrument, (ARG("pos"), ARG("code"), ARG("env")=py::object(), ARG("auto_sp")=true, ARG("append")=true));
    bind_method(obj, "is_unsigned", is_unsigned);
    bind_method(obj, "is_signed", is_signed);    
    bind_method(obj, "is_real", is_real);   
    bind_method(obj, "is_primitive", is_primitive);
    bind_method(obj, "is_array", is_array);    
    bind_method(obj, "is_pointer", is_pointer);
    bind_method(obj, "is_void", is_void);

    bind_method(obj, "is_int", is_int);
    bind_method(obj, "is_short", is_short);    
    bind_method(obj, "is_long", is_long);    
    bind_method(obj, "is_long_long", is_long_long);        
    bind_method(obj, "is_bool", is_bool);        
    bind_method(obj, "is_char", is_char);            
    bind_method(obj, "is_float", is_float);
    bind_method(obj, "is_double", is_double);
    bind_method(obj, "is_string", is_string);

    bind_method(obj, "internal_types", internal_types);        
    bind_method(obj, "find_primitive", find_primitive);        
}

static void instrument(py::object self, std::string pos, std::string code, py::object env, bool auto_sp, bool append) {
    py_instrument(self, pos, code, env, auto_sp, append); 
}

static bool is_unsigned(py::object self) {
    SgType *type = (SgType *) get_ir(self);      
    return type->isUnsignedType();
}

static bool is_signed(py::object self) {
    SgType *type = (SgType *) get_ir(self);      
    return type->isIntegerType();
}

static bool is_real(py::object self) {
    SgType *type = (SgType *) get_ir(self);      
    return type->isFloatType();
}

static bool is_array(py::object self) {
    SgType *type = (SgType *) get_ir(self);      
    return isSgArrayType(type);
}

static bool is_pointer(py::object self) {
    SgType *type = (SgType *) get_ir(self);      
    return isSgPointerType(type);
}

static bool is_void(py::object self) {
    SgType *type = (SgType *) get_ir(self);      
    return isSgTypeVoid(type);
}

static bool is_int(py::object self) {
    SgType *type = (SgType *) get_ir(self);      
    return isSgTypeInt(type);
}

static bool is_short(py::object self) {
    SgType *type = (SgType *) get_ir(self);      
    return isSgTypeShort(type);
}

static bool is_long(py::object self) {
    SgType *type = (SgType *) get_ir(self);      
    return isSgTypeLong(type);
}

static bool is_long_long(py::object self) {
    SgType *type = (SgType *) get_ir(self);      
    return isSgTypeLongLong(type);
}

static bool is_bool (py::object self) {
    SgType *type = (SgType *) get_ir(self);      
    return isSgTypeBool(type);
}

static bool is_char (py::object self) {
    SgType *type = (SgType *) get_ir(self);      
    return isSgTypeChar(type);
}

static bool is_float(py::object self) {
    SgType *type = (SgType *) get_ir(self);      
    return isSgTypeFloat(type);
}

static bool is_double(py::object self) {
    SgType *type = (SgType *) get_ir(self);      
    return isSgTypeDouble(type);
}

static bool is_string(py::object self) {
    SgType *type = (SgType *) get_ir(self);      
    return isSgTypeString(type);
}

static bool is_primitive(py::object self) {
    SgType *type = (SgType *) get_ir(self);      
    return is_int(self) ||
           is_float(self) ||
           is_double(self) ||
           is_bool(self) ||
           is_short(self) ||
           is_long(self) ||
           is_long_long(self) ||
           is_char(self) ||
           is_string(self)
           ;

}



static py::list internal_types(py::object self) {
    SgType *type = (SgType *) get_ir(self);      
    py::list lst = py::list();
    std::vector < SgType * > types = type->getInternalTypes();
    for (auto t : types) {
        lst.append(create_rose_node(t->findBaseType()));
    }       
    return lst;
}

static py::object find_primitive(py::object self) {
    SgType *type = (SgType *) get_ir(self);      
    
    return create_rose_node(type->findBaseType());
}


NODE_SPEC_END



