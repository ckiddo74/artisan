#pragma once

#include <string>
#include <list>
#include <rose.h>

class RoseUtils {
public:
    typedef bool (*FindNodeFn) (SgNode *, void *data, bool &exit);

    static std::string node_loc_str(SgNode *node);
    static std::list<SgNode *> find_sgnodes(SgNode *root, FindNodeFn fn, void *data = 0, int max_depth = -1);  
    static std::string unparse_code(SgNode *node, bool with_updates = true, bool with_pretty_print = true);
};
