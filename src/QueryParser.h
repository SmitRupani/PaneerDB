#ifndef QUERY_PARSER_H
#define QUERY_PARSER_H

#include <statements/SelectStatement.h>
#include <Expression.h>
#include <token.h>
#include <cstddef>
#include <string>
#include <vector>

class QueryParser {

public:
  QueryParser() = default;
  ~QueryParser() = default;

  void tokenize(const std::string &query);

  typedef std::variant<SelectStatement> ParseResult;

  ParseResult parse();

private:
  size_t m_TokenPos;
  std::vector<Token> m_Tokens;

  const Token &consume(TokenType type);
};

#endif
