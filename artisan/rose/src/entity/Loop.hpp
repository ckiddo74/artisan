#pragma once

#include <string>
#include <rose.h>
#include <utils/tinyformat.hpp>
#include <sstream>

int loop_pos(SgNode *parent, SgNode *target_child, enum VariantT variant) {
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

std::string loop_alpha(int pos) {
    std::string ret;

    std::ostringstream sstream;
    sstream << pos;
    
    for (auto c : sstream.str()) {
        ret += 'a' + (9-('9' - c));
    }

    return ret;    
}

std::string loop_tag(SgNode *node, std::string loop_type, enum VariantT variant) {

   while (!isSgFunctionDefinition(node) && node->variantT() != variant)  {
       node = node->get_parent();
   }   


   if (SgFunctionDefinition *fn = isSgFunctionDefinition(node)) {
       return fn->get_declaration()->get_name().getString() + "_" + loop_type; 
   }

   SgNode *parent = node->get_parent();
   
   int pos = loop_pos(parent, node, variant);

   std::string tag = loop_tag(node->get_parent(), loop_type, variant);
   std::string ctag = loop_alpha(pos);

   if (!tag.empty())  {
       tag = tag + "_" + ctag;
   } else {
       tag = ctag;
   }
   
   return tag;
}