#ifndef TYPES_H
#define TYPES_H

#include <string>

enum class TokenType {
  SELECT = 0,
  FROM,
  WHERE,
  OR,
  AND,

  IDENTIFIER,
  STRING_LITERAL,
  NUMERIC_LITERAL,

  EQUALS,
  GREATER_THAN,
  LESS_THAN,
  LPAREN,
  RPAREN,
  END
};

class Token {

public:
  Token(const std::string &valueA, TokenType typeA);

  std::string value;
  TokenType type;

  std::string getTypeName() const;
};

#endif
