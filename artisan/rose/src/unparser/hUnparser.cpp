/***************************************************************************
    hArmonicSDK - Framework for building the hArmonic tool
    Copyright (C) 2010 Jose Gabriel de F. Coutinho (ckiddo74@gmail.com)
 ***************************************************************************/

#include "hUnparser.hpp"
#include <utils/hmsg.hpp>

#include <sstream>
#include <fstream>
#include <algorithm>
#include <unparser.h>

using namespace std;

HUnparserOptions::HUnparserOptions() {
   unparseAll = true;
   withLineNum = false;
}

static string unparseToString ( const SgNode* astNode, HUnparser *HUnparser, SgUnparse_Info *info, const HUnparserOptions &options);


HUnparser::HUnparser() {
	_info = 0;
}

HUnparser::HUnparser(SgUnparse_Info& info) {
   _info = new SgUnparse_Info;
   *_info = info;
}

HUnparser::~HUnparser() {
   delete _info;
}

void HUnparser::setOptions(const HUnparserOptions &options) {
   _options = options;
}

void HUnparser::unparseTraversal(const SgNode *astNode) {
	unparseToString(astNode, this, _info, _options);
}

string HUnparser::toString(const SgNode *astNode, bool removeEmptySpace) {
	string str = unparseToString(astNode, this, _info, _options);
	if (removeEmptySpace) {
	   str = HUnparser::removeWhiteSpace(str);
	}
	return str;
}

void HUnparser::toFile(const SgNode *astNode, string filename) {
	string str = toString(astNode);
	toFile(str, filename);
}

void HUnparser::toFile(string &code, string filename) {
	ofstream myfile(filename.c_str());
	hAssert(!myfile.fail(), "cannot open file [%s] for writing!", filename.c_str());
	myfile << code << endl;
	myfile.close();
}


bool HUnparser::has_instrumentation_code(SgNode *node) {
  return false;
}

string HUnparser::instrument(SgStatement *stmt, SgUnparse_Info& info, const string &code) {
    return code;   
}

string HUnparser::instrument(SgExpression *expr, SgUnparse_Info& info, const string &code) {
    return code;
}

string HUnparser::instrument(SgType *type, SgUnparse_Info& info, const string &code) {
    return code;
}


string HUnparser::removeWhiteSpace(string str, bool left, bool right) {
   hAssert(left || right, "internal error: removeWhitespace parameters left and right are false!");
   string ret;
   size_t startpos = str.find_first_not_of(" \n\t");
   size_t endpos = str.find_last_not_of(" \n\t");
   if(( string::npos == startpos ) || ( string::npos == endpos))  {
      ret = "";
  }  else if (left && right) {
      ret = str.substr( startpos, endpos-startpos+1 );
  } else if (left && (string::npos != startpos )) {
       ret = str.substr( startpos );
  } else if(right && (string::npos != endpos)) {
       ret = str.substr( 0, endpos+1 );
  }
   return ret;
}

template <class T>
static void prepare_instrumentation(HUnparser *hunparser, Unparser *unp, ostringstream *&lstream, UnparseFormat *&original_cursor, UnparseFormat *&lcursor, T node) {
    if (hunparser->has_instrumentation_code(node)) {    
        lstream = new ostringstream;
        original_cursor = new UnparseFormat(lstream, 0); 
    
        memcpy(original_cursor, &unp->cur, sizeof(UnparseFormat));
        lcursor = new UnparseFormat(lstream, 0);
        memcpy(&unp->cur, lcursor, sizeof(UnparseFormat));
    } else {
        lstream = 0;
        original_cursor = 0;
        lcursor = 0;
    }
}

template <class T>
static void finalise_instrumentation(HUnparser *hunparser, Unparser *unp, SgUnparse_Info& info, ostringstream *&lstream, UnparseFormat *&original_cursor, UnparseFormat *&lcursor, T node) {
    if (original_cursor) {    
        string original_code = lstream->str();
        memcpy(&unp->cur, original_cursor, sizeof(UnparseFormat));
        
        string new_code = hunparser->instrument(node, info, original_code);
        unp->cur << new_code;        

        unp->cur.output_stream()->flush();                         

        // don't call destructor, just deallocate
        free(original_cursor);
        free(lcursor);
        free(lstream);

    }   
}

class hExprStmt: public Unparse_ExprStmt {
public:
	hExprStmt(Unparser* unp, std::string fname, HUnparser *hunparser): Unparse_ExprStmt(unp, fname) {
		_hunparser = hunparser;
	}

	virtual void unparseStatement(SgStatement* stmt, SgUnparse_Info& info) {
        ostringstream *lstream;    
        UnparseFormat *original_cursor;
        UnparseFormat *lcursor;    

        prepare_instrumentation(_hunparser, unp, lstream, original_cursor, lcursor, stmt);           
        Unparse_ExprStmt::unparseStatement(stmt, info);
        finalise_instrumentation(_hunparser, unp, info, lstream, original_cursor, lcursor, stmt);    
    }

	virtual void unparseExpression(SgExpression *expr, SgUnparse_Info &info) {
        ostringstream *lstream;    
        UnparseFormat *original_cursor;
        UnparseFormat *lcursor; 

        prepare_instrumentation(_hunparser, unp, lstream, original_cursor, lcursor, expr);           
		Unparse_ExprStmt::unparseExpression(expr, info);
        finalise_instrumentation(_hunparser, unp, info, lstream, original_cursor, lcursor, expr);    
       
	}

protected:
	HUnparser *_hunparser;
};

class hTypes: public Unparse_Type {
public:

	hTypes(Unparser* unp, HUnparser *hunparser): Unparse_Type(unp) {
		_hunparser = hunparser;
        _unp = unp; 
	}

	virtual void unparseType(SgType* type, SgUnparse_Info& info) {
        ostringstream *lstream;    
        UnparseFormat *original_cursor;
        UnparseFormat *lcursor; 

        prepare_instrumentation(_hunparser, _unp, lstream, original_cursor, lcursor, type);           
		Unparse_Type::unparseType(type, info);
        finalise_instrumentation(_hunparser, _unp, info, lstream, original_cursor, lcursor, type);              
	}

protected:
	HUnparser *_hunparser;
    Unparser *_unp; // the base unp is private! (should have been protected!)
};

static string
unparseToString ( const SgNode* astNode, HUnparser *hunparser, SgUnparse_Info* inputUnparseInfoPointer, const HUnparserOptions &options )
   {

     const SgTemplateArgumentPtrList* templateArgumentList = 0;
     const SgTemplateParameterPtrList* templateParameterList = 0;

  // This global function permits any SgNode (including it's subtree) to be turned into a string

  // DQ (9/13/2014): Modified the API to be more general (as part of refactoring support for name qualification).
  // DQ (3/2/2006): Let's make sure we have a valid IR node!
  // ROSE_ASSERT(astNode != NULL);
     ROSE_ASSERT(astNode != NULL || templateArgumentList != NULL || templateParameterList != NULL);

     string returnString;     

#if 0
     if (astNode) {
        printf ("Inside of globalUnparseToString_OpenMPSafe(): astNode = %p = %s, %p \n",astNode,astNode->class_name().c_str(), inputUnparseInfoPointer);
     }
#endif

  // all options are now defined to be false. When these options can be passed in
  // from the prompt, these options will be set accordingly.
     bool _auto                         = false;
     bool linefile                      = options.withLineNum;
     bool useOverloadedOperators        = false;
     bool num                           = false;

  // It is an error to have this always turned off (e.g. pointer = this; will not unparse correctly)
     bool _this                         = true;

     bool caststring                    = false;
     bool _debug                        = false;
     bool _class                        = false;
     bool _forced_transformation_format = false;
     bool _unparse_includes             = options.unparseAll;

#if 0
     printf ("In globalUnparseToString(): astNode->class_name() = %s \n",astNode->class_name().c_str());
#endif

     Unparser_Opt roseOptions( _auto,
                               linefile,
                               useOverloadedOperators,
                               num,
                               _this,
                               caststring,
                               _debug,
                               _class,
                               _forced_transformation_format,
                               _unparse_includes );


  // DQ (7/19/2007): Remove lineNumber from constructor parameter list.
  // int lineNumber = 0;  // Zero indicates that ALL lines should be unparsed

  // Initialize the Unparser using a special string stream inplace of the usual file stream 
     ostringstream outputString;

     const SgLocatedNode* locatedNode = isSgLocatedNode(astNode);
     string fileNameOfStatementsToUnparse;
     if (locatedNode == NULL)
        {
       // printf ("WARNING: applying AST -> string for non expression/statement AST objects \n");
          fileNameOfStatementsToUnparse = "defaultFileNameInGlobalUnparseToString";
        }
       else
        {
          ROSE_ASSERT (locatedNode != NULL);

       // DQ (5/31/2005): Get the filename from a traversal back through the parents to the SgFile
       // fileNameOfStatementsToUnparse = locatedNode->getFileName();
       // fileNameOfStatementsToUnparse = Rose::getFileNameByTraversalBackToFileNode(locatedNode);
          if (locatedNode->get_parent() == NULL)
             {
            // DQ (7/29/2005):
            // Allow this function to be called with disconnected AST fragments not connected to 
            // a previously generated AST.  This happens in Qing's interface where AST fragements 
            // are built and meant to be unparsed.  Only the parent of the root of the AST 
            // fragement is expected to be NULL.
            // fileNameOfStatementsToUnparse = locatedNode->getFileName();
               fileNameOfStatementsToUnparse = locatedNode->getFilenameString();
             }
            else
             {
            // DQ (2/20/2007): The expression being unparsed could be one contained in a SgArrayType
               SgArrayType* arrayType = isSgArrayType(locatedNode->get_parent());
               if (arrayType != NULL)
                  {
                 // If this is an index of a SgArrayType node then handle as a special case
                    fileNameOfStatementsToUnparse = "defaultFileNameInGlobalUnparseToString";
                  }
                 else
                  {
#if 1
                    fileNameOfStatementsToUnparse = Rose::getFileNameByTraversalBackToFileNode(locatedNode);
#else
                    SgSourceFile* sourceFile = TransformationSupport::getSourceFile(locatedNode);
                    ROSE_ASSERT(sourceFile != NULL);
                    fileNameOfStatementsToUnparse = sourceFile->getFileName();
#endif
                  }
             }
        }  // end if locatedNode

     ROSE_ASSERT (fileNameOfStatementsToUnparse.size() > 0);

  // Unparser roseUnparser ( &outputString, fileNameOfStatementsToUnparse, roseOptions, lineNumber );
     Unparser roseUnparser ( &outputString, fileNameOfStatementsToUnparse, roseOptions );
     // Gabriel: Added so that we can instrument code
     string fname = roseUnparser.u_exprStmt->getFileName();     
     delete roseUnparser.u_exprStmt;
     roseUnparser.u_exprStmt = new hExprStmt(&roseUnparser, fname, hunparser);
     delete roseUnparser.u_type;
     roseUnparser.u_type = new hTypes(&roseUnparser, hunparser);

  // Information that is passed down through the tree (inherited attribute)
  // Use the input SgUnparse_Info object if it is available.
     SgUnparse_Info* inheritedAttributeInfoPointer = NULL;

  // DQ (2/18/2013): Keep track of local allocation of the SgUnparse_Info object in this function
  // This is design to fix what appears to be a leak in ROSE (abby-normal growth of the SgUnparse_Info
  // memory pool for compiling large files.
     bool allocatedSgUnparseInfoObjectLocally = false;

     if (inputUnparseInfoPointer != NULL)
        {
       // printf ("Using the input inputUnparseInfoPointer object \n");

       // Use the user provided SgUnparse_Info object
          inheritedAttributeInfoPointer = inputUnparseInfoPointer;
          /* GABE: qualified names should be disabled in C 
          inheritedAttributeInfoPointer->set_SkipQualifiedNames(); */
        }
       else
        {
       // DEFINE DEFAULT BEHAVIOUR FOR THE CASE WHEN NO inputUnparseInfoPointer (== NULL) IS 
       // PASSED AS ARGUMENT TO THE FUNCTION
       // printf ("Building a new Unparse_Info object \n");

       // If no input parameter has been specified then allocate one
       // inheritedAttributeInfoPointer = new SgUnparse_Info (NO_UNPARSE_INFO);
          inheritedAttributeInfoPointer = new SgUnparse_Info();
          ROSE_ASSERT (inheritedAttributeInfoPointer != NULL);

       // DQ (2/18/2013): Keep track of local allocation of the SgUnparse_Info object in this function
          allocatedSgUnparseInfoObjectLocally = true;

       // MS: 09/30/2003: comments de-activated in unparsing
          ROSE_ASSERT (inheritedAttributeInfoPointer->SkipComments() == false);

       // Skip all comments in unparsing
          inheritedAttributeInfoPointer->set_SkipComments();
          ROSE_ASSERT (inheritedAttributeInfoPointer->SkipComments() == true);
       // Skip all whitespace in unparsing (removed in generated string)
          inheritedAttributeInfoPointer->set_SkipWhitespaces();
          ROSE_ASSERT (inheritedAttributeInfoPointer->SkipWhitespaces() == true);

       // Skip all directives (macros are already substituted by the front-end, so this has no effect on those)
          inheritedAttributeInfoPointer->set_SkipCPPDirectives();
          ROSE_ASSERT (inheritedAttributeInfoPointer->SkipCPPDirectives() == true);

#if 1
       // DQ (8/1/2007): Test if we can force the default to be to unparse fully qualified names.
       // printf ("Setting the default to generate fully qualified names, astNode = %p = %s \n",astNode,astNode->class_name().c_str());
          inheritedAttributeInfoPointer->set_forceQualifiedNames();

       // DQ (8/6/2007): Avoid output of "public", "private", and "protected" in front of class members.
       // This does not appear to have any effect, because it it explicitly set in the unparse function 
       // for SgMemberFunctionDeclaration.
          inheritedAttributeInfoPointer->unset_CheckAccess();

       // DQ (8/1/2007): Only try to set the current scope to the SgGlobal scope if this is NOT a SgProject or SgFile
          if ( (isSgProject(astNode) != NULL || isSgFile(astNode) != NULL ) == false )
             {
            // This will be set to NULL where astNode is a SgType!
               inheritedAttributeInfoPointer->set_current_scope(TransformationSupport::getGlobalScope(astNode));
             }
#endif

       // DQ (5/19/2011): Allow compiler generated statements to be unparsed by default.
          inheritedAttributeInfoPointer->set_outputCompilerGeneratedStatements();

       // DQ (1/10/2015): Add initialization of the current_source_file.
       // This is required where this function is called from the name qualification support.



         
        }
        if (astNode) {
          SgSourceFile* sourceFile = TransformationSupport::getSourceFile(astNode);        
          inheritedAttributeInfoPointer->set_current_source_file(sourceFile);
        }   
     ROSE_ASSERT (inheritedAttributeInfoPointer != NULL);
     SgUnparse_Info & inheritedAttributeInfo = *inheritedAttributeInfoPointer;

  // DQ (5/27/2007): Commented out, uncomment when we are ready for Robert's new hidden list mechanism.
  // if (inheritedAttributeInfo.get_current_scope() == NULL)
     if (astNode != NULL && inheritedAttributeInfo.get_current_scope() == NULL)
        {
       // printf ("In globalUnparseToString(): inheritedAttributeInfo.get_current_scope() == NULL astNode = %p = %s \n",astNode,astNode->class_name().c_str());

       // DQ (6/2/2007): Find the nearest containing scope so that we can fill in the current_scope, so that the name qualification can work.
#if 1
          SgStatement* stmt = TransformationSupport::getStatement(astNode);
#else
          SgStatement* stmt = NULL;
       // DQ (6/27/2007): SgProject and SgFile are not contained in any statement
          if (isSgProject(astNode) == NULL && isSgFile(astNode) == NULL)
               stmt = TransformationSupport::getStatement(astNode);
#endif

          if (stmt != NULL)
             {
               SgScopeStatement* scope = stmt->get_scope();
               ROSE_ASSERT(scope != NULL);
               inheritedAttributeInfo.set_current_scope(scope);
             }
            else
             {
            // DQ (6/27/2007): If we unparse a type then we can't find the enclosing statement, so 
            // assume it is SgGlobal. But how do we find a SgGlobal IR node to use?  So we have to 
            // leave it NULL and hand this case downstream!
               inheritedAttributeInfo.set_current_scope(NULL);
             }

          const SgTemplateArgument* templateArgument = isSgTemplateArgument(astNode);
          if (templateArgument != NULL)
             {
            // debugging code!
            // printf ("Exiting to debug case of SgTemplateArgument \n");
            // ROSE_ASSERT(false);

#if 0
            // DQ (9/15/2012): Commented this out since while we build the AST we don't have parents of classes set (until the class declaration is attached to the AST).
               SgScopeStatement* scope = templateArgument->get_scope();
            // printf ("SgTemplateArgument case: scope = %p = %s \n",scope,scope->class_name().c_str());
               inheritedAttributeInfo.set_current_scope(scope);
#else

// DQ (5/25/2013): Commented out this message (too much output spew for test codes, debugging test2013_191.C).
// #ifdef ROSE_DEBUG_NEW_EDG_ROSE_CONNECTION
#if 0
               printf ("Skipping set of inheritedAttributeInfo.set_current_scope(scope); for SgTemplateArgument \n");
#endif
#endif
             }
       // stmt->get_startOfConstruct()->display("In unparseStatement(): info.get_current_scope() == NULL: debug");
       // ROSE_ASSERT(false);
        }
  // ROSE_ASSERT(info.get_current_scope() != NULL);

  // Turn ON the error checking which triggers an error if the default SgUnparse_Info constructor is called
  // SgUnparse_Info::forceDefaultConstructorToTriggerError = true;

  // DQ (10/19/2004): Cleaned up this code, remove this dead code after we are sure that this worked properly
  // Actually, this code is required to be this way, since after this branch the current function returns and
  // some data must be cleaned up differently!  So put this back and leave it this way, and remove the
  // "Implementation Note".

#if 0
     printf ("In globalUnparseToString(): astNode = %p \n",astNode);
     if (astNode != NULL)
        {
          printf ("In globalUnparseToString(): astNode = %p = %s \n",astNode,astNode->class_name().c_str());
        }
#endif

#if 0
     printf ("In globalUnparseToString_OpenMPSafe(): inheritedAttributeInfo.SkipClassDefinition() = %s \n",(inheritedAttributeInfo.SkipClassDefinition() == true) ? "true" : "false");
     printf ("In globalUnparseToString_OpenMPSafe(): inheritedAttributeInfo.SkipEnumDefinition()  = %s \n",(inheritedAttributeInfo.SkipEnumDefinition()  == true) ? "true" : "false");
#endif

  // DQ (1/13/2014): These should have been setup to be the same.
     ROSE_ASSERT(inheritedAttributeInfo.SkipClassDefinition() == inheritedAttributeInfo.SkipEnumDefinition());

  // Both SgProject and SgFile are handled via recursive calls
     if ( (isSgProject(astNode) != NULL) || (isSgSourceFile(astNode) != NULL) )
        {
       // printf ("Implementation Note: Put these cases (unparsing the SgProject and SgFile into the cases for nodes derived from SgSupport below! \n");

       // Handle recursive call for SgProject
          const SgProject* project = isSgProject(astNode);
          if (project != NULL)
             {
               for (int i = 0; i < project->numberOfFiles(); i++)
                  {
                 // SgFile* file = &(project->get_file(i));
                    SgFile* file = project->get_fileList()[i];
                    ROSE_ASSERT(file != NULL);
                    string unparsedFileString = unparseToString(file, hunparser, inputUnparseInfoPointer, options);
                 // string prefixString       = string("/* TOP:")      + string(rose::getFileName(file)) + string(" */ \n");
                 // string suffixString       = string("\n/* BOTTOM:") + string(rose::getFileName(file)) + string(" */ \n\n");
                    string prefixString       = string("/* TOP:")      + file->getFileName() + string(" */ \n");
                    string suffixString       = string("\n/* BOTTOM:") + file->getFileName() + string(" */ \n\n");
                    returnString += prefixString + unparsedFileString + suffixString;
                  }
             }

       // Handle recursive call for SgFile
          const SgSourceFile* file = isSgSourceFile(astNode);
          if (file != NULL)
             {
               SgGlobal* globalScope = file->get_globalScope();
               ROSE_ASSERT(globalScope != NULL);
               returnString = unparseToString(globalScope, hunparser, inputUnparseInfoPointer, options);
             }
        }
       else
        {
       // DQ (1/12/2003): Only now try to trap use of SgUnparse_Info default constructor
       // Turn ON the error checking which triggers an error if the default SgUnparse_Info constructor is called
       // GB (09/27/2007): Took this out because it breaks parallel traversals that call unparseToString. It doesn't
       // seem to have any other effect (whatever was debugged with this seems to be fixed now).
       // SgUnparse_Info::set_forceDefaultConstructorToTriggerError(true);

          if (isSgStatement(astNode) != NULL)
             {
               const SgStatement* stmt = isSgStatement(astNode);

            // DQ (9/6/2010): Added support to detect use of C (default) or Fortran code.
            // DQ (2/2/2007): Note that we should modify the unparser to take the IR nodes as const pointers, but this is a bigger job than I want to do now!
            // roseUnparser.u_exprStmt->unparseStatement ( const_cast<SgStatement*>(stmt), inheritedAttributeInfo );
               if (SageInterface::is_Fortran_language() == true)
                  {
                 // Unparse as a Fortran code.
                    ROSE_ASSERT(roseUnparser.u_fortran_locatedNode != NULL);
                    roseUnparser.u_fortran_locatedNode->unparseStatement ( const_cast<SgStatement*>(stmt), inheritedAttributeInfo );
                  }
                 else
                  {
                 // Unparse as a C/C++ code.
#if 0
                    printf ("In globalUnparseToString_OpenMPSafe(): calling roseUnparser.u_exprStmt->unparseStatement() \n");
#endif
                    ROSE_ASSERT(roseUnparser.u_exprStmt != NULL);

                 // printf ("Calling roseUnparser.u_exprStmt->unparseStatement() stmt = %s \n",stmt->class_name().c_str());
                 // roseUnparser.u_exprStmt->curprint ("Output from curprint");
                    roseUnparser.u_exprStmt->unparseStatement ( const_cast<SgStatement*>(stmt), inheritedAttributeInfo );
#if 0
                    printf ("In globalUnparseToString_OpenMPSafe(): DONE: calling roseUnparser.u_exprStmt->unparseStatement() \n");
#endif
                  }
             }

          if (isSgExpression(astNode) != NULL)
             {
               const SgExpression* expr = isSgExpression(astNode);

            // DQ (9/6/2010): Added support to detect use of C (default) or Fortran code.
            // DQ (2/2/2007): Note that we should modify the unparser to take the IR nodes as const pointers, but this is a bigger job than I want to do now!
            // roseUnparser.u_exprStmt->unparseExpression ( const_cast<SgExpression*>(expr), inheritedAttributeInfo );
               if (SageInterface::is_Fortran_language() == true)
                  {
                 // Unparse as a Fortran code.
                    ROSE_ASSERT(roseUnparser.u_fortran_locatedNode != NULL);
                    roseUnparser.u_fortran_locatedNode->unparseExpression ( const_cast<SgExpression*>(expr), inheritedAttributeInfo );
                  }
                 else
                  {
                 // Unparse as a C/C++ code.
                    ROSE_ASSERT(roseUnparser.u_exprStmt != NULL);
                    roseUnparser.u_exprStmt->unparseExpression ( const_cast<SgExpression*>(expr), inheritedAttributeInfo );
                  }
             }

          if (isSgType(astNode) != NULL)
             {
               const SgType* type = isSgType(astNode);

#if 0
               printf ("In globalUnparseToString_OpenMPSafe(): inheritedAttributeInfo.SkipClassDefinition() = %s \n",(inheritedAttributeInfo.SkipClassDefinition() == true) ? "true" : "false");
               printf ("In globalUnparseToString_OpenMPSafe(): inheritedAttributeInfo.SkipEnumDefinition()  = %s \n",(inheritedAttributeInfo.SkipEnumDefinition()  == true) ? "true" : "false");
#endif
            // DQ (1/13/2014): These should have been setup to be the same.
               ROSE_ASSERT(inheritedAttributeInfo.SkipClassDefinition() == inheritedAttributeInfo.SkipEnumDefinition());

            // DQ (9/6/2010): Added support to detect use of C (default) or Fortran code.
            // DQ (2/2/2007): Note that we should modify the unparser to take the IR nodes as const pointers, but this is a bigger job than I want to do now!
#if 1
               ROSE_ASSERT(roseUnparser.u_type != NULL);
               roseUnparser.u_type->unparseType ( const_cast<SgType*>(type), inheritedAttributeInfo );
#else
               if (SageInterface::is_Fortran_language() == true)
                  {
                 // Unparse as a Fortran code.
                    roseUnparser.u_fortran_locatedNode->unparseType ( const_cast<SgType*>(type), inheritedAttributeInfo );
                  }
                 else
                  {
                 // Unparse as a C/C++ code.
                    roseUnparser.u_type->unparseType ( const_cast<SgType*>(type), inheritedAttributeInfo );
                  }
#endif
             }

          if (isSgSymbol(astNode) != NULL)
             {
               const SgSymbol* symbol = isSgSymbol(astNode);

            // DQ (2/2/2007): Note that we should modify the unparser to take the IR nodes as const pointers, but this is a bigger job than I want to do now!
               ROSE_ASSERT(roseUnparser.u_sym != NULL);
               roseUnparser.u_sym->unparseSymbol ( const_cast<SgSymbol*>(symbol), inheritedAttributeInfo );
             }

          if (isSgSupport(astNode) != NULL)
             {
            // Handle different specific cases derived from SgSupport 
            // (e.g. template parameters and template arguments).
               switch (astNode->variantT())
                  {
#if 0
                    case V_SgProject:
                       {
                         SgProject* project = isSgProject(astNode);
                         ROSE_ASSERT(project != NULL);
                         for (int i = 0; i < project->numberOfFiles(); i++)
                            {
                              SgFile* file = &(project->get_file(i));
                              ROSE_ASSERT(file != NULL);
                              string unparsedFileString = globalUnparseToString_OpenMPSafe(file,inputUnparseInfoPointer);
                              string prefixString       = string("/* TOP:")      + string(Rose::getFileName(file)) + string(" */ \n");
                              string suffixString       = string("\n/* BOTTOM:") + string(Rose::getFileName(file)) + string(" */ \n\n");
                              returnString += prefixString + unparsedFileString + suffixString;
                            }
                         break;
                       }
#error "DEAD CODE!"
                 // case V_SgFile:
                       {
                         SgFile* file = isSgFile(astNode);
                         ROSE_ASSERT(file != NULL);
                         SgGlobal* globalScope = file->get_globalScope();
                         ROSE_ASSERT(globalScope != NULL);
                         returnString = globalUnparseToString_OpenMPSafe(globalScope,inputUnparseInfoPointer);
                         break;
                       }
#endif
                    case V_SgTemplateParameter:
                       {
                         const SgTemplateParameter* templateParameter = isSgTemplateParameter(astNode);

                      // DQ (2/2/2007): Note that we should modify the unparser to take the IR nodes as const pointers, but this is a bigger job than I want to do now!
                         ROSE_ASSERT(roseUnparser.u_exprStmt != NULL);
                         roseUnparser.u_exprStmt->unparseTemplateParameter(const_cast<SgTemplateParameter*>(templateParameter),inheritedAttributeInfo);
                         break;
                       }

                    case V_SgTemplateArgument:
                       {
                         const SgTemplateArgument* templateArgument = isSgTemplateArgument(astNode);
#if 0
                      // printf ("In globalUnparseToString_OpenMPSafe(): case V_SgTemplateArgument (before): returnString = %s outputString = %s \n",returnString.c_str(),outputString.str());
                         printf ("In globalUnparseToString_OpenMPSafe(): case V_SgTemplateArgument (before): returnString = %s outputString = %s \n",returnString.c_str(),outputString.str().c_str());
                      // printf ("In globalUnparseToString_OpenMPSafe(): case V_SgTemplateArgument (before): returnString = %s \n",returnString.c_str());
#endif
                      // DQ (2/2/2007): Note that we should modify the unparser to take the IR nodes as const pointers, but this is a bigger job than I want to do now!
                         ROSE_ASSERT(roseUnparser.u_exprStmt != NULL);
                         roseUnparser.u_exprStmt->unparseTemplateArgument(const_cast<SgTemplateArgument*>(templateArgument),inheritedAttributeInfo);

                      // printf ("In globalUnparseToString_OpenMPSafe(): case V_SgTemplateArgument (after): returnString = %s outputString = %s \n",returnString.c_str(),outputString.str());
                      // printf ("In globalUnparseToString_OpenMPSafe(): case V_SgTemplateArgument (after): returnString = %s outputString = %s \n",returnString.c_str(),outputString.str());
                      // printf ("In globalUnparseToString_OpenMPSafe(): case V_SgTemplateArgument (after): returnString = %s \n",returnString.c_str());

#if 0
                         string local_returnString = outputString.str();
                         printf ("In globalUnparseToString_OpenMPSafe(): case V_SgTemplateArgument (after): local_returnString = %s \n",local_returnString.c_str());
#endif
                         break;
                       }

                    case V_Sg_File_Info:
                       {
                      // DQ (8/5/2007): This is implemented above as a special case!
                      // DQ (5/11/2006): Not sure how or if we should implement this
                         break;
                       }

                    case V_SgPragma:
                       {
                         const SgPragma* pr = isSgPragma(astNode);
                         SgPragmaDeclaration* decl = isSgPragmaDeclaration(pr->get_parent());
                         ROSE_ASSERT (decl);
                         ROSE_ASSERT(roseUnparser.u_exprStmt != NULL);
                         roseUnparser.u_exprStmt->unparseStatement ( decl, inheritedAttributeInfo );
                         break;
                       }

                    case V_SgFileList:
                       {
                      // DQ (1/23/2010): Not sure how or if we should implement this
                         const SgFileList* fileList = isSgFileList(astNode);
                         ROSE_ASSERT(fileList != NULL);
#if 0
                         for (int i = 0; i < project->numberOfFiles(); i++)
                            {
                              SgFile* file = &(project->get_file(i));
                              ROSE_ASSERT(file != NULL);
                              string unparsedFileString = globalUnparseToString_OpenMPSafe(file,inputUnparseInfoPointer);
                              string prefixString       = string("/* TOP:")      + string(Rose::getFileName(file)) + string(" */ \n");
                              string suffixString       = string("\n/* BOTTOM:") + string(Rose::getFileName(file)) + string(" */ \n\n");
                              returnString += prefixString + unparsedFileString + suffixString;
                            }
#else
                         printf ("WARNING: SgFileList support not implemented for unparser...\n");
#endif
                         break;
                       }

                 // Perhaps the support for SgFile and SgProject shoud be moved to this location?
                    default:
                       {
                         printf ("Error: default reached in node derived from SgSupport astNode = %s \n",astNode->class_name().c_str());
                         ROSE_ABORT();
                       }
                  }
             }

          if (astNode == NULL)
             {
            // DQ (9/13/2014): This is where we could put support for when astNode == NULL, and the input was an STL list of IR node pointers.
               if (templateArgumentList != NULL)
                  {
#if 0
                    printf ("Detected SgTemplateArgumentPtrList: templateArgumentList = %p size = %zu \n",templateArgumentList,templateArgumentList->size());
#endif
                    roseUnparser.u_exprStmt->unparseTemplateArgumentList(*templateArgumentList, inheritedAttributeInfo );
#if 0
                    printf ("Exiting as a test! \n");
                    ROSE_ASSERT(false);
#endif
                  }

               if (templateParameterList != NULL)
                  {
#if 0
                    printf ("Detected SgTemplateParameterPtrList: templateParameterList = %p size = %zu \n",templateParameterList,templateParameterList->size());
#endif
                    roseUnparser.u_exprStmt->unparseTemplateParameterList(*templateParameterList, inheritedAttributeInfo );
#if 0
                    printf ("Exiting as a test! \n");
                    ROSE_ASSERT(false);
#endif
                  }
             }

       // Liao 11/5/2010 move out of SgSupport
          if (isSgInitializedName(astNode)) //       case V_SgInitializedName:
             {
            // DQ (8/6/2007): This should just unparse the name (fully qualified if required).
            // QY: not sure how to implement this
            // DQ (7/23/2004): This should unparse as a declaration (type and name with initializer).
               const SgInitializedName* initializedName = isSgInitializedName(astNode);
            // roseUnparser.get_output_stream() << initializedName->get_qualified_name().str();
               SgScopeStatement* scope = initializedName->get_scope();
               if (isSgGlobal(scope) == NULL && scope->containsOnlyDeclarations() == true)
                  {
                    roseUnparser.get_output_stream() << roseUnparser.u_exprStmt->trimGlobalScopeQualifier ( scope->get_qualified_name().getString() ) << "::";
                  }
               roseUnparser.get_output_stream() << initializedName->get_name().str();
            // break;
             }


       // Liao, 8/28/2009, support for SgLocatedNodeSupport
          if (isSgLocatedNodeSupport(astNode) !=  NULL) 
             {
               if (isSgOmpClause(astNode))
                  {
                    SgOmpClause * omp_clause = const_cast<SgOmpClause*>(isSgOmpClause(astNode));
                    ROSE_ASSERT(omp_clause);

                    ROSE_ASSERT(roseUnparser.u_exprStmt != NULL);
                    roseUnparser.u_exprStmt->unparseOmpClause(omp_clause, inheritedAttributeInfo);
                  }
             }

       // Turn OFF the error checking which triggers an if the default SgUnparse_Info constructor is called
       // GB (09/27/2007): Removed this error check, see above.
       // SgUnparse_Info::set_forceDefaultConstructorToTriggerError(false);

       // MS: following is the rewritten code of the above outcommented 
       //     code to support ostringstream instead of ostrstream.
          returnString = outputString.str();

       // Call function to tighten up the code to make it more dense
          if (inheritedAttributeInfo.SkipWhitespaces() == true)
             {
               returnString = roseUnparser.removeUnwantedWhiteSpace ( returnString );
             }
        }

  // delete the allocated SgUnparse_Info object
     if (inputUnparseInfoPointer == NULL)
        {
          delete inheritedAttributeInfoPointer;
          inheritedAttributeInfoPointer = NULL;
        }

#if 0
     printf ("In globalUnparseToString_OpenMPSafe(): returnString = %s \n",returnString.c_str());
#endif

  // DQ (2/18/2013): Keep track of local allocation of the SgUnparse_Info object in this function
     if (allocatedSgUnparseInfoObjectLocally == true)
        {
          ROSE_ASSERT(inheritedAttributeInfoPointer == NULL);
        }

     return returnString;
   }

