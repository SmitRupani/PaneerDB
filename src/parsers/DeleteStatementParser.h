#ifndef DELETE_STATEMENT_PARSER_H
#define DELETE_STATEMENT_PARSER_H

#include "StatementParser.h"
#include "../statements/DeleteStatement.h"
#include <vector>

class Expression;

class DeleteStatementParser : public StatementParser {
public:
  explicit DeleteStatementParser(std::vector<Token> &tokens);

  DeleteStatement *parse();

private:
  Expression *parseExpr();
  Expression *parseOr();
  Expression *parseAnd();
  Expression *parseTerm();
};

#endif
