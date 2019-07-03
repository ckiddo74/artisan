#include "Entity.hpp"
#include <boost/regex.hpp>

using namespace std;

EntityInfo::EntityInfo(CheckFn chfn, CreateFn crfn, MetaFn mtfn, enum VariantT vt) {
    check_fn = chfn;
    create_fn = crfn;
    meta_fn = mtfn;
    meta_rank = -1;
    sg_variant = vt; 
}

void EntityInfo::compute_meta() {
    (*meta_fn)(this);
}


namespace entity {

list<pair<string, string> > Entity::convert_params(string params) {

    list<pair<string, string>> ret_params;

    if (params.empty()) return ret_params;

    

/*
    // replaces C++ method parameters to Python 
    s = boost::regex_replace(s, boost::regex("^\\((.+)\\)$"), "$1");
    s = boost::regex_replace(s, boost::regex("boost::python::arg\\(\"([^\"]+)\"\\)"), "$1");
    s = boost::regex_replace(s, boost::regex("=[ ]*boost::python::object\\(\\)"), "=None");
    s = boost::regex_replace(s, boost::regex("=[ ]*true"), "=True");
    s = boost::regex_replace(s, boost::regex("=[ ]*false"), "=False"); 


    // now find all occurrences of ARG(...)
    const regex rarg("ARG\\((.+)\\)?");
    smatch marg;
    if (regex_search(args, marg, rarg)) {
        for (int i = 1; i < marg.size(); i++) {
            printf("---> %s\n", marg[i].str().c_str());
        }
    }
*/

      // remove the enclosing ( )
      //printf("params[init: %s]\n", params.c_str());
      params = boost::regex_replace(params, boost::regex("^\\((.+)\\)$"), "$1");
      //printf("...params[next: %s]\n", params.c_str());
      params = boost::regex_replace(params, boost::regex(",[\\w: ]+::object\\(\\)"), ", None");
      //printf("...params[next: %s]\n", params.c_str());
      params = boost::regex_replace(params, boost::regex(",[ ]*true"), ", True");
      //printf("...params[next: %s]\n", params.c_str());
      params = boost::regex_replace(params, boost::regex(",[ ]*false"), ", False"); 
      //printf("params[end: %s]\n", params.c_str());
      
      // parsing: ARG("match"), ARG("where", ""), ARG("env", None)
      boost::regex re("ARG\\(\"(\\w+)\"[ ]*([,][ ]*([^\\)]+))?\\)");
      boost::sregex_token_iterator i(params.begin(), params.end(), re, {1, 3});
      boost::sregex_token_iterator j;

      while(i != j)
      {
         pair<string, string> ret_param;
         ret_param.first = *i++;
         ret_param.second = *i++;
         ret_params.push_back(ret_param);
      }

    return ret_params;
}

}