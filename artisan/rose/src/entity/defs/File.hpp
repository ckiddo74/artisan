#pragma once

#include "Node.hpp"

ENTITY_SPEC_BEGIN(File, "AST node representing a source file", SgSourceFile, Node, node, obj, entity, sg_type) {

    std::string name;

    if (node) {
        name = node->getFileName();
    } 
    
    bind_attr(obj, "name", "source-file name", name);
    bind_method(obj, "tag", "source-file name", tag);  

    bind_method(obj, "parent", "gets parent", parent);
}

static std::string tag(SgNodePtr self) {
     SgSourceFile *node = isSgSourceFile(self);

     return node->getFileName();
}     


static py::object parent(py::object obj) {
    SgSourceFile *node = (SgSourceFile *) to_sgnode(obj);   
    return create_rose_node(node->get_project());
} 
   
ENTITY_SPEC_END
