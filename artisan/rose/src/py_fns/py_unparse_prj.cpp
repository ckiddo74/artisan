#include <rose.h>
#include "py_unparse_prj.hpp"
#include <utils/hmsg.hpp>
#include <utils/rose_utils.hpp>
#include <py_sgnode.hpp>

using namespace std;

void __unparse_prj(boost::python::object project, bool pretty_print) {
    assert_entity(project, "Project");

    SgProject *prj = (SgProject *) to_sgnode(project);

    const SgStringList sources = prj->get_sourceFileNameList();

    // write files
    int i = 0;

    list<string> newFiles;   
    i = 0;
    for (string source: sources) {
        hAssert(boost::filesystem::exists(source), "[%s] does not exist - cannot overwrite!", source.c_str());
        SgSourceFile *file = isSgSourceFile((*prj) [i]);
        hAssert(file, "wrong type of file: %s", source.c_str());
        hlog(1, "unparser", "   * injecting woven code into %s", source.c_str());

        std::ofstream oFile; oFile.open(source.c_str());
        oFile << RoseUtils::unparse_code(file, true, pretty_print) << endl;          
        oFile.close(); 
        i++;
   }
}