#include <parsers/StatementParser.h>
#include <stdexcept>
#include <string>

StatementParser::StatementParser(std::vector<Token> &tokens)
    : m_Tokens(tokens), m_Pos(0) {}

const Token &StatementParser::peek() const {
  if (m_Pos >= m_Tokens.size()) {
    throw std::runtime_error("[StatementParser] Unexpected end of tokens");
  }
  return m_Tokens[m_Pos];
}

const Token &StatementParser::consume(TokenType expected) {
  const Token &t = peek();
  if (t.type != expected) {
    throw std::runtime_error(std::string("[StatementParser] Expected token: ") +
                             t.getTypeName(expected) + ", got: " + t.getTypeName());
  }
  ++m_Pos;
  return t;
}

bool StatementParser::accept(TokenType type) {
  if (m_Pos < m_Tokens.size() && m_Tokens[m_Pos].type == type) {
    ++m_Pos;
    return true;
  }
  return false;
}
