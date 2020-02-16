#pragma once

#include "Type.h"
#include <instrumentation/py_instrument.h>

NODE_SPEC_BEGIN(ArrayType, SgArrayType, Type, node, obj) {
    bind_method(obj, "index", index);
    bind_method(obj, "base", base);

}

static py::object index(py::object self) {
    SgArrayType *type = (SgArrayType *) get_ir(self);  
    return create_rose_node(type->get_index());
}

static py::object base(py::object self) {
    SgArrayType *type = (SgArrayType *) get_ir(self);  
    return create_rose_node(type->get_base_type());
}

NODE_SPEC_END



