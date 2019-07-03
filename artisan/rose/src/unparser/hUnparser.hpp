/***************************************************************************
    hArmonicSDK - Framework for building the hArmonic tool
    Copyright (C) 2010 Jose Gabriel de F. Coutinho (ckiddo74@gmail.com)
 ***************************************************************************/

#ifndef _HUNPARSER_H_
#define _HUNPARSER_H_

#include <rose.h>
#include <string>

class HUnparserOptions {
public:
   HUnparserOptions();
   
   bool unparseAll;
   bool withLineNum;   
};

class HUnparser {
	friend class hExprStmt;
	friend class hSymbols;
	friend class hTypes;

public:
	HUnparser();
	HUnparser(SgUnparse_Info& info);
	virtual ~HUnparser();
	
	void setOptions(const HUnparserOptions &options);

	void unparseTraversal(const SgNode *astNode);
	std::string toString(const SgNode *astNode, bool removeEmptySpace = true );
	void toFile(const SgNode *astNode, std::string filename);
	void toFile(std::string &code, std::string filename);

    virtual bool has_instrumentation_code(SgNode *node);	

	virtual std::string instrument(SgStatement *stmt, SgUnparse_Info& info, const std::string &code);
	virtual std::string instrument(SgExpression *expr, SgUnparse_Info& info, const std::string &code);
	virtual std::string instrument(SgType *type, SgUnparse_Info& info, const std::string &code);
     
	static std::string removeWhiteSpace(std::string str, bool left = true, bool right = true);	

protected:
	SgUnparse_Info *_info;
	HUnparserOptions _options;
};

#endif /* HUnparser_H_ */


