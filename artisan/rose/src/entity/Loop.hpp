#pragma once

#include <string>
#include <rose.h>
#include <list>
#include <utils/tinyformat.hpp>
#include <utils/rose_utils.hpp>
#include <sstream>

class LoopUtils {
private:
static int __loop_pos(SgNode *parent, SgNode *target_child, enum VariantT variant) {
    int i = 0;
    int pos = 0;
    SgScopeStatement *_parent = isSgScopeStatement(parent);
    if (_parent) {
        size_t num_children = _parent->get_numberOfTraversalSuccessors();
        for (int i = 0; i < num_children; i++) {
            SgNode *child = _parent->get_traversalSuccessorByIndex(i);
            if (child == target_child) {
                break;
            } else {
                if (child->variantT() == variant) {
                    pos++;
                }
            }
        }
    }

    return pos;    
}

static std::string __loop_alpha(int pos) {
    std::string ret;

    std::ostringstream sstream;
    sstream << pos;
    
    for (auto c : sstream.str()) {
        ret += 'a' + (9-('9' - c));
    }

    return ret;    
}

public:
static std::string loop_tag(SgNode *node, std::string loop_type, enum VariantT variant) {

   while (!isSgFunctionDefinition(node) && node->variantT() != variant)  {
       node = node->get_parent();
   }   


   if (SgFunctionDefinition *fn = isSgFunctionDefinition(node)) {
       return fn->get_declaration()->get_name().getString() + "_" + loop_type; 
   }

   SgNode *parent = node->get_parent();
   
   int pos = __loop_pos(parent, node, variant);

   std::string tag = LoopUtils::loop_tag(node->get_parent(), loop_type, variant);
   std::string ctag = __loop_alpha(pos);

   if (!tag.empty())  {
       tag = tag + "_" + ctag;
   } else {
       tag = ctag;
   }
   
   return tag;
}

static bool is_outermost(SgNode *node, bool for_loop, bool while_loop, bool do_loop) {
    SgNode *parent = node->get_parent();

    // loops until it finds a loop that meets loop type
    while (parent && ![for_loop, while_loop, do_loop](SgNode *node) -> bool  {             
            return (node->variantT() == V_SgForStatement && for_loop) || 
                   (node->variantT() == V_SgWhileStmt && while_loop) ||
                   (node->variantT() == V_SgDoWhileStmt && do_loop)            
            ; 
        } (parent)  ) {
           parent = parent->get_parent();
    }

    return parent == NULL;
}

static bool test(SgNode *, bool &exit);

static bool is_innermost(SgNode *node, bool for_loop, bool while_loop, bool do_loop) {

    struct LoopType {
        bool for_loop;
        bool while_loop;
        bool do_loop;
        SgNode *node;
    };

    LoopType lt = { for_loop, while_loop, do_loop, node };

    std::list<SgNode *> nodes = RoseUtils::find_sgnodes(node, [](SgNode *node, void *data, bool &exit) -> bool {
           LoopType *_data = (LoopType *) data;
           if (_data->node != node) { // we don't care about the parent node
                if ( (node->variantT() == V_SgForStatement && _data->for_loop) || 
                    (node->variantT() == V_SgWhileStmt && _data->while_loop) ||
                    (node->variantT() == V_SgDoWhileStmt && _data->do_loop) ) {
                        exit = true;
                        return true;
                    }               
                }      
           return false; 

        }, &lt, -1);

    // if it is empty, then it is innermost
    return nodes.empty();

}

};
