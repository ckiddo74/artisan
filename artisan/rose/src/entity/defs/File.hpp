#pragma once

#include "Node.hpp"

ENTITY_SPEC_BEGIN(File, "AST node representing a source file", SgSourceFile, Node, node, obj, entity, sg_type) {
    // obj.attr("name") = node->getFileName();

    // base name
    //obj.attr("tag") = py::str(boost::filesystem::path (node->getFileName()).filename().string());

    bind_method(obj, "parent", "gets parent", parent);
}

static py::object parent(py::object obj) {
    SgSourceFile *node = (SgSourceFile *) to_sgnode(obj);   
    return create_rose_node(node->get_project());
} 
   
ENTITY_SPEC_END
