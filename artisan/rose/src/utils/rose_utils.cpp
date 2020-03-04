#include <utils/rose_utils.hpp>
#include <utils/tinyformat.hpp>
#include <unparser/hUnparserEx.hpp>
#include <unparser/hbcpp.hpp>

using namespace std;

string RoseUtils::node_loc_str (SgNode *node) {
	string str;

	SgLocatedNode *lnode = isSgLocatedNode(node);

	if (lnode) {
		Sg_File_Info *info = lnode->get_file_info();
		string file = info->get_filename();
		int line = info->get_line();
		int col = info->get_col();

        str = tfm::format("[%s (%d,%d)]",  file, line, col);
	}
	return str;
}


list<SgNode *> RoseUtils::find_sgnodes(SgNode *root, RoseUtils::FindNodeFn fn, int max_depth) {
    struct NodeFinder: public AstTopDownProcessing<int> {
       int _max_depth;
       FindNodeFn _fn;
       std::list<SgNode *> _nodes;
       
	    NodeFinder(int max_depth, FindNodeFn fn) {
            _max_depth = max_depth;
            _fn = fn;
        }  
      
       int evaluateInheritedAttribute(SgNode *node, int depth) {  
          if (_max_depth == -1 || depth <= _max_depth) {
            bool exit=false;  
            if (_fn(node, exit)) {
                _nodes.push_back(node);
            }
            if (exit) {
                throw 0;
            }
          }
          return depth + 1;  
       }
    };

    NodeFinder nf(max_depth, fn);
    try {
        nf.traverse(root, 0);
    } catch (int x) {
    }

    return nf._nodes;
}

string RoseUtils::unparse_code(SgNode *node, bool with_updates, bool with_pretty_print) {
    if (!isSgProject(node) && !isSgSourceFile(node) && isSgSupport(node)) return "";
    SgUnparse_Info inputUnparseInfoPointer;
    inputUnparseInfoPointer.unset_SkipComments();    // generate comments
    inputUnparseInfoPointer.unset_SkipWhitespaces(); // generate all whitespaces to format the code      
    inputUnparseInfoPointer.unset_SkipCPPDirectives();
    
    HUnparser *unparser; 

    if (with_updates) {
        unparser = new HUnparserEx(inputUnparseInfoPointer);
    } else {
        unparser = new HUnparser(inputUnparseInfoPointer);
    }
   
    HUnparserOptions unparseOptions; unparseOptions.unparseAll = false; 
    unparser->setOptions(unparseOptions);     
    
    if (with_pretty_print) {
       return hbcpp(unparser->toString(node));
    } else {
       return unparser->toString(node);
    }   
    delete unparser;
}

/************
 * list<SgNode *>
FunctionCallNormalization::BFSQueryForNodes( SgNode *root, VariantT type )
{
  list<SgNode *> toVisit, retList;
  toVisit.push_back( root );

  while ( !toVisit.empty() )
    {
      SgNode *crt = toVisit.front();
      if ( crt->variantT() == type )
        retList.push_back( crt );
      
      vector<SgNode *> succ = ( crt )->get_traversalSuccessorContainer();
      for ( vector<SgNode *>::iterator succIt = succ.begin(); succIt != succ.end(); succIt++ )
        if ( isSgNode ( *succIt ) )
          toVisit.push_back( *succIt );
      
      toVisit.pop_front();
    }
  return retList;
}
*/