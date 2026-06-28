#ifndef TRANSACTION_STATEMENTS_H
#define TRANSACTION_STATEMENTS_H

#include "Statement.h"
#include <iostream>

class BeginStatement : public Statement {
public:
  BeginStatement() : Statement(StatementType::BEGIN_TRANSACTION) {}
  void print() const override {
    std::cout << "BEGIN;\n";
  }
};

class CommitStatement : public Statement {
public:
  CommitStatement() : Statement(StatementType::COMMIT_TRANSACTION) {}
  void print() const override {
    std::cout << "COMMIT;\n";
  }
};

class RollbackStatement : public Statement {
public:
  RollbackStatement() : Statement(StatementType::ROLLBACK_TRANSACTION) {}
  void print() const override {
    std::cout << "ROLLBACK;\n";
  }
};

#endif
