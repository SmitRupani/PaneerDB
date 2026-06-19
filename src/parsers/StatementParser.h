#ifndef STATEMENT_PARSER_H
#define STATEMENT_PARSER_H

#include <token.h>
#include <vector>

class StatementParser {
public:
  explicit StatementParser(std::vector<Token> &tokens);
  virtual ~StatementParser() = default;

protected:
  std::vector<Token> &m_Tokens;
  size_t m_Pos;

  const Token &peek() const;
  const Token &consume(TokenType expected);
  bool accept(TokenType type);
};

#endif
