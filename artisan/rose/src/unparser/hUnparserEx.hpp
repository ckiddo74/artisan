/***************************************************************************
    hArmonicSDK - Framework for building the hArmonic tool
    Copyright (C) 2010 Jose Gabriel de F. Coutinho (ckiddo74@gmail.com)
 ***************************************************************************/

#ifndef __HUNPARSER_EX_H_
#define __HUNPARSER_EX_H_

#include "hUnparser.hpp"
#include <set>
#include <boost/python.hpp>

class HUnparserEx: public HUnparser {

public:

class RewriteAttr: public AstAttribute {
public:
   struct Spec {
      std::string code;
      bool append;
      boost::python::object env;
      Spec() { code = "";  append = false; env = boost::python::object();}
   };

   typedef enum { rp_before, rp_replace, rp_after, rp_begin, rp_end  } Position;
   RewriteAttr();
   
   void register_spec(Position pos, const Spec &spec);
   bool get_spec(Position pos, Spec &spec);         
   void clear();  
   void disable();
   void enable();
   bool is_enabled();

   virtual OwnershipPolicy getOwnershipPolicy() const ROSE_OVERRIDE { return CONTAINER_OWNERSHIP; }
   
protected:
   // pos -> spec
   std::map<Position, Spec> _specs;
   bool _enabled;

};   

public:
	HUnparserEx();
    HUnparserEx(SgUnparse_Info& info);
	virtual ~HUnparserEx();
	void init();
	static RewriteAttr* query_spec(SgNode *node);
	static void register_spec(SgNode *node, RewriteAttr::Position pos, const RewriteAttr::Spec &spec);

protected:
    virtual bool has_instrumentation_code(SgNode *node);	
    virtual std::string eval_code(SgNode *node, const RewriteAttr::Spec &spec);
	virtual std::string instrument(SgStatement *stmt, SgUnparse_Info& info, const std::string &code);
	virtual std::string instrument(SgExpression *expr, SgUnparse_Info& info, const std::string &code);
	virtual std::string instrument(SgType *type, SgUnparse_Info& info, const std::string &code);

};


#endif 


