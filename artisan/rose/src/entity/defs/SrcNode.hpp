#pragma once

#include <string>
#include <fstream>
#include <streambuf>

#include "Node.hpp"
#include <utils/hmsg.hpp>
#include <unparser/hUnparserEx.hpp>
#include <py_fns/py_instrument.hpp>

ENTITY_SPEC_BEGIN(SrcNode, "AST nodes that have source locations", SgLocatedNode, Node, node, obj, entity, sg_type) {
#if 0    
    obj.attr("filename") = node->get_file_info()->get_filenameString();
    obj.attr("line") = node->get_file_info()->get_line();
    obj.attr("col") = node->get_file_info()->get_col();
#endif
    bind_method(obj, "instrument", "inject code in node (pos=[before/around/after])", instrument, (ARG("pos"), ARG("code"), ARG("env", py::object()), ARG("auto_sp", true), ARG("append", true)));
    bind_method(obj, "in_src", "", in_src);
/*
    DEF_PROPERTY(obj, "col",  (int64_t) node, 
        "returns the source column number")   
*/        
}

static void instrument(py::object self, std::string pos, std::string code, py::object env, bool auto_sp, bool append) {
    py_instrument(self, pos, code, env, auto_sp, append); 
}

static bool in_src(py::object self) {
    static SgProject *prj_cache = 0;
    static std::set<std::string> prj_srcs;

    SgLocatedNode *node = (SgLocatedNode *) to_sgnode(self);   
    PrjAttribute *attr = (PrjAttribute *) node->getAttribute("project");
    hAssert(attr, "(internal error) expecting python ROSE node [%s] to have an SG attribute called 'project'!", node->class_name());
    SgProject *prj = attr->prj();

    // add the list of filenames in the cache
    if (prj != prj_cache) {
        prj_cache = prj;
        for (int i = 0; i < prj->numberOfFiles(); i++) {
            prj_srcs.insert(prj->get_file(i).getFileName());
        }
    }

    std::string src_filename = node->get_file_info()->get_filenameString();

    return prj_srcs.count(src_filename) != 0;
}
   
ENTITY_SPEC_END



