#ifndef SELECT_STATEMENT_H
#define SELECT_STATEMENT_H

#include <vector>
#include <string>
#include "Expression.h"

class SelectStatement {
public:
  SelectStatement(std::vector<std::string> projectionA,
                  std::string tableNameA, Expression *filterA);

  void print() const;

  std::vector<std::string> projection;
  std::string tableName;
  Expression *filter;
};

#endif