#include "hbcpp.hpp"

using namespace std;

namespace bcpp {
   string bcpp(string src);
}
  
string hbcpp(string src) {
   if (src.empty()) {return "";}
   else { return bcpp::bcpp(src); }
}
   
