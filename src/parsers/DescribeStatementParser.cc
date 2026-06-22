#include <parsers/DescribeStatementParser.h>

DescribeStatementParser::DescribeStatementParser(std::vector<Token> &tokens) : StatementParser(tokens) {}

DescribeStatement* DescribeStatementParser::parse() {
  consume(TokenType::DESC);
  std::string tableName = consume(TokenType::IDENTIFIER).value;
  return new DescribeStatement(std::move(tableName));
}
