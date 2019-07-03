#include <stdio.h>
#include <boost/spirit/home/qi.hpp>
#include "boost/spirit/include/phoenix.hpp"
#include <boost/algorithm/string/trim.hpp>
#include <set>
#include "query_grammar.hpp"
#include <utils/hmsg.hpp>

/**********************************
 *  => x:abc =(3)>  def{what} =(..)> ghi =(1..)> jkl =(..4)> m 
 * 
 * edge := "=>"
 * node := [a..zA..Z_] 
 * term := edge node
 * query := edge? node term*
 * /
 * 
*/

using namespace std;


BOOST_FUSION_ADAPT_STRUCT(rose_query_grammar::query_node_t, 
   (string, var)
   (string, entity)
   (string, tag)
)

BOOST_FUSION_ADAPT_STRUCT(rose_query_grammar::query_edge_t, 
   (uint, min)
   (uint, max)
)


namespace rose_query_grammar {

namespace phoenix = boost::phoenix;
namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;



template <typename Iterator>
struct rose_model_query_grammar : qi::grammar<Iterator, query_t(), ascii::space_type>
{
    rose_model_query_grammar() : rose_model_query_grammar::base_type(query)
    {
       using qi::alpha;
       using qi::alnum;        
       using qi::char_;
       using qi::_val;        
       using qi::_1;  
       using qi::uint_;
       using phoenix::ref;
       using phoenix::at_c;
       using phoenix::val;
     
       text = qi::lexeme[+(char_ - '}' - '{')];

       id = (qi::alpha | qi::char_("_"))
              >> *(qi::alnum | qi::char_( "_"));
       
       edge = 
              // =n:m> 
              (qi::lit("=") >> uint_ [at_c<0>(_val) = _1] >> qi::lit(":") >> uint_ [at_c<1>(_val) = _1] >> qi::lit(">") ) |
              // =:m> 
              (qi::lit("=") >> qi::lit(":") >> uint_ [at_c<0>(_val) = val(0), at_c<1>(_val) = _1] >> qi::lit(">") ) |
             // =n:> 
              (qi::lit("=") >> uint_ [at_c<0>(_val) = _1, at_c<1>(_val) = val(0)] >> qi::lit(":") >> qi::lit(">") ) |             
             // =n>
              (qi::lit("=") >>  uint_ [at_c<0>(_val) = _1, at_c<1>(_val) = _1] >> qi::lit(">") ) |
              // =:> 
              (qi::lit("=") >> qi::lit(":") >> qi::lit(">") [at_c<0>(_val) = val(0), at_c<1>(_val) = val(0)])  |              
              // =>
              (qi::lit("=>") [ at_c<0>(_val) = val(0), at_c<1>(_val) = val(0) ]); 
              
       node =  
           // var:entity{tag}
           (id [ at_c<0>(_val) = _1 ] >> ":" >> id [ at_c<1>(_val) = _1] >> "{" >> text [at_c<2>(_val) = _1 ] >> "}") |
           // entity {tag}
           (id [ at_c<0>(_val) = val(""), at_c<1>(_val) = _1] >> "{" >> text [at_c<2>(_val) = _1 ] >> "}") |
           // var:entity
           (id [ at_c<0>(_val) = _1 ] >> ":" >> id [ at_c<1>(_val) = _1, at_c<2>(_val) = val("")  ]) | 
           // entity
           (id [ at_c<0>(_val) = val(""), at_c<1>(_val) = _1, at_c<2>(_val) = val("") ]);
       
       query = -edge [ phoenix::push_back(phoenix::bind(&query_t::edges, _val), _1) ] >>
               node [ phoenix::push_back(phoenix::bind(&query_t::nodes, _val), _1) ] >>
               *(
                   edge [ phoenix::push_back(phoenix::bind(&query_t::edges, _val), _1) ] |
                   node [ phoenix::push_back(phoenix::bind(&query_t::nodes, _val), _1) ]
               );
             
   
    }

    qi::rule<Iterator, query_node_t(), ascii::space_type> node;
    qi::rule<Iterator, query_edge_t(), ascii::space_type> edge;
    qi::rule<Iterator, query_t(), ascii::space_type> query;
    qi::rule<Iterator, string()> id;    
    qi::rule<Iterator, string()> text;    
    qi::rule<Iterator, int> digits;

  
};

void normalise(query_t &query) {
    list<query_node_t> &nodes = query.nodes;
    list<query_edge_t> &edges = query.edges;

    size_t nn = nodes.size();
    size_t ne = edges.size();

    // same number of nodes and edges
    if (ne == nn - 1) {
        edges.push_front(query_edge_t({0, 0}));
        ne = edges.size();
    } 

    hEXCEPTION_IF(ne != nn, "malformed query [%s]: #edges (%d) must be equal to #nodes (%d)!", 
                    query.query, ne, nn);

    // make sure variable names are not the same, and remove
    // tag whitespace
    set<string> vars;
    for (auto &n : nodes) {
        // check if variable is empty, in that case, give the same name as entity
        //if (n.var == "") n.var = n.entity;
        
        // check if variable has been defined in query
        hEXCEPTION_IF(n.var != "" && vars.count(n.var) > 0, "invalid query [%s]: variable [%s] already defined!", 
                    query.query, n.var);

        // trim tag
        boost::algorithm::trim (n.tag);
        vars.insert(n.var);
    }

    // check edge distances
    for (auto &e : edges) {
        hEXCEPTION_IF(e.min > e.max, "invalid query [%s]: min edge distance (%d) larger than max edge distance (%d)", 
                    query.query, e.min, e.max);            
    }
}            

void parse(string query_str, query_t &query) {
  
   string::iterator beg = query_str.begin();
   string::iterator end = query_str.end();
   rose_model_query_grammar<string::iterator> grammar;

   bool r = phrase_parse(beg, end, grammar, ascii::space, query);
   query.query = query_str;

   hEXCEPTION_IF(!r || beg != end, "malformed query [%s]!", query_str);

   normalise(query);
   
}

void print (const query_t &query) {
   hlog(2, "query", "=== QUERY: %s ====", query.query);
   for (auto &x : query.nodes) {
      hlog(2, "query", "[var:'%s'], [entity:'%s'], [tag:'%s']", x.var.c_str(), x.entity.c_str(), x.tag.c_str());
    } 
    for (auto &x : query.edges) {
       hlog(2, "query", "[min:%d] => [max:%d]", x.min, x.max);
    }   
   hlog(2, "query", "===========================");
}


}
/*
using namespace rose_query_grammar;

int main(int argc, char *argv[]) {
    
   string x = "default";
   if (argc > 1) x = argv[1];
   query_t query;

   parse(x, query);
      
   
   for (auto x : query.nodes) {
     printf("[var:%s], [entity:%s], [tag: %s]\n", x.var.c_str(), x.entity.c_str(), x.tag.c_str());
   } 
   for (auto x : query.edges) {
      printf("[min:%d] => [max:%d]\n", x.min, x.max);
   } 
   
   return 0;
}
*/
