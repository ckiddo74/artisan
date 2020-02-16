/***************************************************************************
    hArmonicSDK - Framework for building the hArmonic tool
    Copyright (C) 2010 Jose Gabriel de F. Coutinho (ckiddo74@gmail.com)
 ***************************************************************************/

#include "hUnparserEx.hpp"
#include <utils/hmsg.hpp>
#include <string>
#include <map>
#include <boost/regex.hpp>
#include <py_sgnode.hpp>

namespace py = boost::python;

using namespace std;

HUnparserEx::HUnparserEx() {

}



HUnparserEx::HUnparserEx(SgUnparse_Info& info): HUnparser(info) { }

HUnparserEx::~HUnparserEx() { }

HUnparserEx::RewriteAttr::RewriteAttr() { 
    enable();
}

bool HUnparserEx::RewriteAttr::is_enabled() {
    return _enabled;
}

void HUnparserEx::RewriteAttr::disable() {
    _enabled = false;
}

void HUnparserEx::RewriteAttr::enable() {
    _enabled = true;
}

void HUnparserEx::RewriteAttr::register_spec(Position pos, const Spec &spec) {
   Spec &s = _specs[pos];
   std::string code = spec.code;
   if (spec.append) {
       code = s.code + code;
   }
   s.code = code;
   s.env = spec.env;
}

bool HUnparserEx::RewriteAttr::get_spec(Position pos, HUnparserEx::RewriteAttr::Spec &spec) {
   map<Position, Spec>::iterator s = _specs.find(pos);
   if (s == _specs.end()) return false;
   spec = s->second;
   return true;
}

void HUnparserEx::RewriteAttr::clear() {
   _specs.clear();
}

HUnparserEx::RewriteAttr* HUnparserEx::query_spec(SgNode *node) {
   hAssert(isSgLocatedNode(node) || isSgType(node), "internal error: invalid node - cannot manipulate rspecs!");
   RewriteAttr *rs = static_cast<RewriteAttr *>(node->getAttribute("RewriteAttr"));
   if (rs && rs->is_enabled()) { return rs; }
   return 0;
}

void HUnparserEx::register_spec(SgNode *node, HUnparserEx::RewriteAttr::Position pos, const HUnparserEx::RewriteAttr::Spec &spec) {
   hAssert(isSgLocatedNode(node) || isSgType(node), "internal error: invalid node - cannot manipulate rspecs!");
   RewriteAttr *rs = static_cast<RewriteAttr *>(node->getAttribute("RewriteAttr"));
   if (!rs) {
      rs = new RewriteAttr();
      node->setAttribute("RewriteAttr", rs);      
   }
   rs->register_spec(pos, spec);
}


bool HUnparserEx::has_instrumentation_code(SgNode *node) {
    return query_spec(node) != 0;
}

string HUnparserEx::eval_code(SgNode *node, const RewriteAttr::Spec &spec) {

    string _code = boost::regex_replace(spec.code, boost::regex("\\{@[ ]*([^@]+)@\\}"), [node, &spec] (const boost::smatch &m) -> string
        {
            py::dict scope = py::dict();
            if (spec.env != py::object()) {
                py::dict d = py::extract<py::dict>(spec.env);
                scope = d.copy();     
            }
            scope["_"] = create_rose_node(node);
 
            string code_to_eval =  m[1].str();
            RewriteAttr *rs = query_spec(node); 
            rs->disable(); // prevent from rewriting the node (infinite recursion)
            py::object res = py::eval(code_to_eval.c_str(),  scope);   
            rs->enable();
            return py::extract<string>(res);
        });   
   
    return _code;
}

// TODO: Possibly remove rp_begin and rp_end

/* statement instrumentation: before replace { begin ... end } after */
string HUnparserEx::instrument(SgStatement *stmt, SgUnparse_Info& info, const string &code) {    
   HUnparserEx::RewriteAttr *rs = query_spec(stmt);
   string instr_str = code;

   if (rs) {
      HUnparserEx::RewriteAttr::Spec spec;
      if (rs->get_spec(HUnparserEx::RewriteAttr::rp_replace, spec)) {
         instr_str = eval_code(stmt, spec);
      }
      if (rs->get_spec(HUnparserEx::RewriteAttr::rp_before, spec)) {
         instr_str = eval_code(stmt, spec) + instr_str;
      }
      if (rs->get_spec(HUnparserEx::RewriteAttr::rp_after, spec)) {
         instr_str = instr_str + eval_code(stmt, spec);
      }
      if (rs->get_spec(HUnparserEx::RewriteAttr::rp_begin, spec)) {          
          SgBasicBlock *block = isSgBasicBlock(stmt);
          if (block) {
            std::size_t pos = instr_str.find_first_of("{");
            string before, after;
            if (pos != std::string::npos) {
               before = instr_str.substr(0, pos);
               after = instr_str.substr(pos+1);         
               instr_str = before + "{" + eval_code(stmt, spec) + after;
            }
          } else {
             SgGlobal *global = isSgGlobal(stmt);
             if (global) {
                instr_str = eval_code(stmt, spec) + instr_str;
             } else {
                hAssert(0, "invalid instrumentation directive <begin> in a non-block/global construct!");
             }
          }
      }
      if (rs->get_spec(HUnparserEx::RewriteAttr::rp_end, spec)) {
          SgBasicBlock *block = isSgBasicBlock(stmt);

          if (block) {          
            std::size_t pos = instr_str.find_last_of("}");
            string before, after;
            if (pos != std::string::npos) {
               before = instr_str.substr(0, pos);
               after = instr_str.substr(pos+1);         
               instr_str = before + eval_code(stmt, spec) + "}" + after;
            }
          } else {
             SgGlobal *global = isSgGlobal(stmt);
             if (global) {
                instr_str = instr_str + eval_code(stmt, spec);
             } else {
                hAssert(0, "invalid instrumentation directive <end> in a non-block/global construct!");
             }             
          }
      }

   }

    return instr_str;
}

/* expression instrumentation: <before> <replace> <after> */
string HUnparserEx::instrument(SgExpression *expr, SgUnparse_Info& info, const std::string &code) {
   HUnparserEx::RewriteAttr *rs = query_spec(expr);
   string instr_str = code;

   if (rs) {
      HUnparserEx::RewriteAttr::Spec spec;
      if (rs->get_spec(HUnparserEx::RewriteAttr::rp_replace, spec)) {
         instr_str = eval_code(expr, spec);
      }
      if (rs->get_spec(HUnparserEx::RewriteAttr::rp_before, spec)) {
         instr_str = eval_code(expr, spec) + instr_str;
      }
      if (rs->get_spec(HUnparserEx::RewriteAttr::rp_after, spec)) {
         instr_str = instr_str + eval_code(expr, spec);
      }
   }

   return instr_str;
}

/* Type instrumentation: Before <sym> After */
string HUnparserEx::instrument(SgType *type, SgUnparse_Info& info, const string &code) {
   // rewrite rules
   HUnparserEx::RewriteAttr *rs = query_spec(type);
   string instr_str = code;

   if (rs) {
      HUnparserEx::RewriteAttr::Spec spec;
      if (rs->get_spec(HUnparserEx::RewriteAttr::rp_before, spec) && info.isTypeFirstPart()) {
         instr_str = eval_code(type, spec);
      }
      if (rs->get_spec(HUnparserEx::RewriteAttr::rp_after, spec) && info.isTypeSecondPart()) {
         instr_str = eval_code(type, spec);
      }
   }
   return instr_str;
}

#if 0
bool HUnparserEx::stmt(SgStatement *stmt, std::string &str) {
   bool modified = false;

// TODO: add stronger verification scheme
  size_t idx0;
  do {
     idx0 = str.find("__unnamed_class___");
     if (idx0 == string::npos) {
        idx0 = str.find("__unnamed_enum___");
     }
     if (idx0 != string::npos) {
        size_t idxN = str.find_first_of(' ', idx0);
        str = str.substr(0, idx0) + str.substr(idxN);
        modified = true;
      }
   } while (idx0 != string::npos);

// Fix multiline pragmas
   if (isSgPragmaDeclaration(stmt)) {
      string newStr;
      bool changed = false;
      for (size_t i = 0; i < str.size(); i++) {
      	if (str[i] == '\n') {
      		newStr += " \\\n";
      		changed = true;
      	} else {
      		newStr += str[i];
      	}
      }
      if (changed) {
      	str = newStr;
      	modified = true;
      }
   }
#if 0
// fix the switch case, where a { } is added to every switch case - we exclude if it already has
   if (stmt && isSgBasicBlock(stmt) && (isSgCaseOptionStmt(stmt->get_parent()) || isSgDefaultOptionStmt(stmt->get_parent()))) {
      Rose_STL_Container<SgStatement*> stmts = ((SgBasicBlock *) stmt)->getStatementList();
      if (stmts.size() == 1 && isSgBasicBlock(stmts[0])) {
         size_t p0 = str.find_first_of("{");
         size_t p1 = str.find_last_of("}");
         if (p0 != string::npos && p1 != string::npos && p1 > p0) {
            str = str.substr(p0+1, p1-p0-1);
            modified = true;
         }
      }
   }

// fix the switch statement, where a { } is added at top-level to every switch statement
   if (stmt && isSgBasicBlock(stmt) && !isSgFunctionDefinition(stmt->get_parent())) {
      Rose_STL_Container<SgStatement*> stmts = ((SgBasicBlock *) stmt)->getStatementList();
      if (stmts.size() == 1 && isSgSwitchStatement(stmts[0])) {
         size_t p0 = str.find_first_of("{");
         size_t p1 = str.find_last_of("}");
         if (p0 != string::npos && p1 != string::npos && p1 > p0) {
            str = str.substr(p0+1, p1-p0-1);
            modified = true;
         }
      }
   }
#endif
   // rewrite rules
   RewriteAttr *rs = queryRSpec(stmt);

   if (rs) {
      string code;
      code = rs->getCode(RewriteAttr::rp_around);
      if (!code.empty()) {
         str = code;
         modified = true;
      }
      code = rs->getCode(RewriteAttr::rp_before);
      if (!code.empty()) {
         str = code + "\n" + str;
         modified = true;
      }
      code = rs->getCode(RewriteAttr::rp_after);
      if (!code.empty()) {
         str = str + "\n" + code;
         modified = true;
      }

      if (isSgScopeStatement(stmt)) {
         // begin
         code = rs->getCode(RewriteAttr::rp_begin_before);
         if (!code.empty()) {
            size_t pos = str.find_first_of("{");

            if (pos != string::npos) {
               string before = str.substr(0, pos);
               string after = str.substr(pos);
               str = before + code + after;
               modified = true;
            }
         }
         code = rs->getCode(RewriteAttr::rp_begin_around);
         if (!code.empty()) {

            size_t pos = str.find_first_of("{");
            if (pos != string::npos) {
               string before = str.substr(0, pos);
               string after = str.substr(pos+1);
               str = before + code + after;
               modified = true;
            }
         }
         code = rs->getCode(RewriteAttr::rp_begin_after);
         if (!code.empty()) {
            size_t pos = str.find_first_of("{");
            if (pos != string::npos) {
               string before = str.substr(0, pos+1);
               string after = str.substr(pos+1);
               str = before + code + after;
               modified = true;
            }
         }

         // end
         code = rs->getCode(RewriteAttr::rp_end_before);
         if (!code.empty()) {
            size_t pos = str.find_last_of("}");

            if (pos != string::npos) {
               string before = str.substr(0, pos);
               string after = str.substr(pos);
               str = before + code + after;
               modified = true;
            }
         }
         code = rs->getCode(RewriteAttr::rp_end_around);
         if (!code.empty()) {

            size_t pos = str.find_last_of("}");
            if (pos != string::npos) {
               string before = str.substr(0, pos);
               string after = str.substr(pos+1);
               str = before + code + after;
               modified = true;
            }
         }
         code = rs->getCode(RewriteAttr::rp_end_after);
         if (!code.empty()) {
            size_t pos = str.find_last_of("}");
            if (pos != string::npos) {
               string before = str.substr(0, pos+1);
               string after = str.substr(pos+1);
               str = before + code + after;
               modified = true;
            }
         }


      }
   }

   return modified;
}

bool HUnparserEx::preExpr(SgExpression *expr, std::string &str) {
	return false;
}

// TODO: Need to expand to all headers.
bool HUnparserEx::expr(SgExpression *expr, std::string &str) {
   bool modified = false;

#if 0
   /* handle all __BB_<name> vars or functions */
   if (isSgVarRefExp(expr) || isSgFunctionRefExp(expr)) {
      string prefix = "__BB_"; size_t size_prefix = prefix.size();
      if (str.substr(0, size_prefix) == prefix) {
         str = str.substr(size_prefix);
         modified = true;
      }
   }

   /* handle builtin functions */
   if (isSgFunctionCallExp(expr)) {
      if (SgFunctionRefExp *refExp = isSgFunctionRefExp(((SgFunctionCallExp *) expr)->get_function())) {
         string ref = refExp->get_symbol()->get_name();
         map<string, string>::iterator f = builtinMap.find(ref);
         if (f != builtinMap.end()) {
            str = f->second;
            modified= true;
         }
      }
   }
#endif

#if 0
   // fix cast where it is sometimes placed next to another with the same cast - e.g. (int *) (int *)
   if (isSgCastExp(expr) && isSgCastExp(((SgCastExp *) expr)->get_operand())) {
      SgCastExp *cast1 = (SgCastExp *) expr;
      SgCastExp *cast2 = (SgCastExp *) ((SgCastExp *) expr)->get_operand();
      string tcast1 = cast1->get_type()->unparseToString();
      string tcast2 = cast2->get_type()->unparseToString();

      if (!tcast1.empty() && tcast1 == tcast2 && !cast1->get_file_info()->isCompilerGenerated() && !cast2->get_file_info()->isCompilerGenerated()) {
         size_t p0 = str.find_first_of("(");
         size_t p1 = str.find_first_of(")");
      /*   printf(":::> %s: (%s, %s, %d, %d):(%p, %p) => %s\n", tcast1.c_str(), cast1->unparseToString().c_str(), cast2->unparseToString().c_str(),
         cast1->get_file_info()->isCompilerGenerated(), cast2->get_file_info()->isCompilerGenerated(),
         cast1, cast2, str.c_str()); */

         if (p0 != string::npos && p1 != string::npos && p1 > p0) {
            string cast = str.substr(p0+1, p1-p0-1);
            if (cast == tcast1) {
                 str = str.substr(p1+1);
                 modified=true;
            }
         }
      }
   }
#endif

   RewriteAttr *rs = queryRSpec(expr);
   if (rs) {
      string code;
      code = rs->getCode(RewriteAttr::rp_around);
      if (!code.empty()) {
         str = code;
         modified = true;
      }
      code = rs->getCode(RewriteAttr::rp_before);
      if (!code.empty()) {
         str = code + "\n" + str;
         modified = true;
      }
      code = rs->getCode(RewriteAttr::rp_after);
      if (!code.empty()) {
         str = str + "\n" + code;
         modified = true;
      }
   }

	return modified;
}

#endif