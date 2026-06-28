#ifndef DELETE_STATEMENT_H
#define DELETE_STATEMENT_H

#include "Statement.h"
#include <string>

class Expression;

class DeleteStatement : public Statement {
public:
  DeleteStatement();
  ~DeleteStatement() override;

  void print() const override;

  std::string tableName;
  Expression *filter;
};

#endif
