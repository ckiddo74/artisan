#pragma once

#include <list>
#include <string>

namespace rose_query_grammar { 
           
struct query_node_t {
   std::string var;
   std::string entity;
   std::string tag;
};

struct query_edge_t {
   uint min;
   uint max;
};

struct query_t
{
   std::list<query_node_t> nodes;
   std::list<query_edge_t> edges;
   std::string query;
};

void parse(std::string query_str, query_t &query);
void print(const query_t &query);

}

