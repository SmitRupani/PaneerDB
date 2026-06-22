#ifndef DESCRIBE_STATEMENT_H
#define DESCRIBE_STATEMENT_H

#include "statements/Statement.h"
#include <string>
#include <utility>

class DescribeStatement : public Statement {
public:
  explicit DescribeStatement(std::string tableNameA)
      : Statement(Statement::StatementType::DESCRIBE_TABLE),
        tableName(std::move(tableNameA)) {}
  ~DescribeStatement() override = default;

  void print() const override {
    // Empty
  }

  std::string tableName;
};

#endif // DESCRIBE_STATEMENT_H
