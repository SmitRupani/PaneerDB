#include "DeleteStatementParser.h"
#include "../Expression.h"
#include <stdexcept>
#include <string>

DeleteStatementParser::DeleteStatementParser(std::vector<Token> &tokens)
    : StatementParser(tokens) {}

DeleteStatement *DeleteStatementParser::parse() {
  consume(TokenType::DELETE_KW);
  consume(TokenType::FROM);
  std::string tableName = consume(TokenType::IDENTIFIER).value;

  Expression *filter = nullptr;
  if (accept(TokenType::WHERE)) {
    filter = parseExpr();
  }

  if (m_Pos < m_Tokens.size() && m_Tokens[m_Pos].type == TokenType::END) {
    ++m_Pos;
  }

  auto *stmt = new DeleteStatement();
  stmt->tableName = tableName;
  stmt->filter = filter;
  return stmt;
}

Expression *DeleteStatementParser::parseExpr() { return parseOr(); }

Expression *DeleteStatementParser::parseOr() {
  Expression *left = parseAnd();

  while (m_Pos < m_Tokens.size() && m_Tokens[m_Pos].type == TokenType::OR) {
    ++m_Pos;
    Expression *right = parseAnd();
    left = new OrExpression(left, right);
  }

  return left;
}

Expression *DeleteStatementParser::parseAnd() {
  Expression *left = parseTerm();

  while (m_Pos < m_Tokens.size() && m_Tokens[m_Pos].type == TokenType::AND) {
    ++m_Pos;
    Expression *right = parseTerm();
    left = new AndExpression(left, right);
  }

  return left;
}

Expression *DeleteStatementParser::parseTerm() {
  const Token &t = peek();
  if (t.type == TokenType::LPAREN) {
    consume(TokenType::LPAREN);
    Expression *inner = parseExpr();
    consume(TokenType::RPAREN);
    return inner;
  }

  Token leftTok = consume(TokenType::IDENTIFIER);
  Expression *leftExpr = new IdentifierExpression(leftTok.value);

  TokenType op = peek().type;

  if (op == TokenType::EQUALS || op == TokenType::GREATER_THAN ||
      op == TokenType::LESS_THAN) {
    ++m_Pos;

    const Token &rightTok = peek();
    Expression *rightExpr = nullptr;

    if (rightTok.type == TokenType::IDENTIFIER) {
      rightExpr =
          new IdentifierExpression(consume(TokenType::IDENTIFIER).value);
    } else if (rightTok.type == TokenType::NUMERIC_LITERAL) {
      std::string v = consume(TokenType::NUMERIC_LITERAL).value;
      int iv = 0;
      try {
        iv = std::stoi(v);
      } catch (...) {
        throw std::runtime_error("[DeleteParser] Invalid numeric literal");
      }
      rightExpr = new LiteralExpression(iv);
    } else if (rightTok.type == TokenType::STRING_LITERAL) {
      std::string s = consume(TokenType::STRING_LITERAL).value;
      rightExpr = new LiteralExpression(s);
    } else {
      throw std::runtime_error("[DeleteParser] Unexpected token in comparison");
    }

    if (op == TokenType::EQUALS) {
      return new EqualityExpression(leftExpr, rightExpr);
    } else if (op == TokenType::GREATER_THAN) {
      return new GreaterExpression(leftExpr, rightExpr);
    } else {
      return new LessExpression(leftExpr, rightExpr);
    }
  }

  throw std::runtime_error("[DeleteParser] Expected comparison operator");
}
