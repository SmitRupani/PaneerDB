#include <Expression.h>
#include <parsers/InsertStatementParser.h>
#include <stdexcept>
#include <string>

InsertStatementParser::InsertStatementParser(std::vector<Token> &tokens)
    : StatementParser(tokens) {}

InsertStatement *InsertStatementParser::parse() {
  consume(TokenType::INSERT);
  consume(TokenType::INTO);

  std::string tableName = consume(TokenType::IDENTIFIER).value;

  std::vector<std::string> columns;

  if (accept(TokenType::LPAREN)) {
    columns.push_back(consume(TokenType::IDENTIFIER).value);
    while (accept(TokenType::COMMA)) {
      columns.push_back(consume(TokenType::IDENTIFIER).value);
    }
    consume(TokenType::RPAREN);
  }

  consume(TokenType::VALUES);
  consume(TokenType::LPAREN);

  std::vector<Expression *> values;

  auto parseValue = [this]() -> Expression * {
    Token t = peek();
    if (t.type == TokenType::STRING_LITERAL) {
      return new LiteralExpression(consume(TokenType::STRING_LITERAL).value);
    } else if (t.type == TokenType::NUMERIC_LITERAL) {
      std::string v = consume(TokenType::NUMERIC_LITERAL).value;
      return new LiteralExpression(std::stoi(v));
    } else {
      throw std::runtime_error("[InsertParser] Invalid value type");
    }
  };

  values.push_back(parseValue());

  while (accept(TokenType::COMMA)) {
    values.push_back(parseValue());
  }

  consume(TokenType::RPAREN);

  if (m_Pos < m_Tokens.size() && m_Tokens[m_Pos].type == TokenType::END) {
    ++m_Pos;
  }

  return new InsertStatement(std::move(tableName), std::move(columns),
                             std::move(values));
}
