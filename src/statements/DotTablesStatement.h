#ifndef DOT_TABLES_STATEMENT_H
#define DOT_TABLES_STATEMENT_H

#include "statements/Statement.h"
#include <iostream>

class DotTablesStatement : public Statement {
public:
  DotTablesStatement() : Statement(Statement::StatementType::DOT_TABLES) {}
  ~DotTablesStatement() override = default;

  void print() const override {
    // Empty
  }
};

#endif // DOT_TABLES_STATEMENT_H
