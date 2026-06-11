#ifndef QUERY_PARSER_H
#define QUERY_PARSER_H

#include "types.h"
#include <vector>

class QueryParser {

public:
  QueryParser() = default;
  ~QueryParser() = default;

  std::vector<Token> tokenize(const std::string &query);
};

#endif
