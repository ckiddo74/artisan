#pragma once

#include <string>
#include <fstream>
#include <streambuf>

#include "Node.hpp"
#include <utils/hmsg.hpp>
#include <unparser/hUnparserEx.hpp>
#include <py_fns/py_instrument.hpp>
#include <boost/filesystem.hpp>

ENTITY_SPEC_BEGIN(NodeSrc, "AST nodes that have source locations", SgLocatedNode, Node, node, obj, entity, sg_type) {
    bind_method(obj, "instrument", "inject code in node (pos=[before/replace/after])", instrument, (ARG("pos"), ARG("code"), ARG("env", py::object()), ARG("auto_sp", true), ARG("append", true)));
    bind_method(obj, "in_code", "whether it is part of the input source code (.cpp)", in_code);
    bind_method(obj, "coords", "return construct coordinates as tuple (filename, line, column) - use full_path=True to return full path", coords, ARG("full_path", false))
}

static void instrument(py::object self, std::string pos, std::string code, py::object env, bool auto_sp, bool append) {
    py_instrument(self, pos, code, env, auto_sp, append); 
}

static bool in_code(py::object self) {
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

static py::tuple coords(py::object self, bool full_path) {
    SgLocatedNode *node = (SgLocatedNode *) to_sgnode(self);   

    std::string path = node->get_file_info()->get_filenameString();

    if (!full_path) {
        path = boost::filesystem::path(path.c_str()).filename().string();
    }

    py::tuple tuple = py::make_tuple(path, 
                                     node->get_file_info()->get_line(), 
                                     node->get_file_info()->get_col());
    return tuple;
}
   
ENTITY_SPEC_END



