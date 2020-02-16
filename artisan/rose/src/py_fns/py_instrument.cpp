#include <rose.h>
#include "py_instrument.hpp"
#include <utils/hmsg.hpp>
#include <unparser/hUnparserEx.hpp>
#include <py_sgnode.hpp>

using namespace std;
namespace py = boost::python;

void py_instrument(py::object self, string where, string code, py::object env, bool auto_sp, bool append) {

      SgNode *node = (SgNode *) to_sgnode(self);

      hAssert(isSgLocatedNode(node) || isSgType(node), 
          "internal error: cannot instrument node: not a source construct!");   

      // argument validation
      hAssert(!isSgBasicBlock(node) || where == "before" || where == "after" || where == "replace" || 
               where == "begin" || where == "end",
             "block can only be instrumented in 'before', 'after', 'replace', 'begin' or 'end' position!");

      hAssert(!isSgGlobal(node) || where == "before" || where == "after" || where == "replace" || 
               where == "begin" || where == "end",
             "global can only be instrumented in 'before', 'after', 'replace', 'begin' or 'end' position!");             

      hAssert(!isSgType(node) || where == "before" || where == "after",
             "types can only be instrumented in 'before' or 'after' position!");

      hAssert(isSgBasicBlock(node) || isSgGlobal(node) || !isSgLocatedNode(node) || where == "before" || where == "after" || where == "replace",
             "source construct can only be instrumented in 'before', 'after' or 'replace' position!");


      HUnparserEx::RewriteAttr::Position pos;     
      if (where == "before") {
         pos = HUnparserEx::RewriteAttr::rp_before; 
      } else if (where == "replace") {
         pos = HUnparserEx::RewriteAttr::rp_replace;
      } else if  (where == "after") { 
         pos = HUnparserEx::RewriteAttr::rp_after;
      } else if (where == "begin") {
         pos = HUnparserEx::RewriteAttr::rp_begin;
      } else if (where == "end") {
         pos = HUnparserEx::RewriteAttr::rp_end;
      }
   
   HUnparserEx::RewriteAttr::Spec spec;
   
   if (auto_sp) {
      if (isSgType(node))  {
          if (where == "before") {
             spec.code = code + " ";
          } else {
             spec.code = " " + code;
          }   
      } else if (isSgLocatedNode(node)) {
          if (!isSgExpression(node)) {
             spec.code = "\n" + code + "\n";
          } else {
             spec.code = code;
          }
      }
   } else {
       spec.code = code;
   }

   spec.append = append;
   spec.env = env;
   HUnparserEx::register_spec(node, pos, spec);
}
