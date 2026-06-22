#ifndef DESCRIBE_STATEMENT_PARSER_H
#define DESCRIBE_STATEMENT_PARSER_H

#include <parsers/StatementParser.h>
#include <statements/DescribeStatement.h>

class DescribeStatementParser : public StatementParser {
public:
  explicit DescribeStatementParser(std::vector<Token> &tokens);
  ~DescribeStatementParser() override = default;

  DescribeStatement* parse();
};

#endif // DESCRIBE_STATEMENT_PARSER_H
