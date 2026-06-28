#include "DeleteStatement.h"
#include "Expression.h"
#include <iostream>

DeleteStatement::DeleteStatement()
    : Statement(StatementType::DELETE_STATEMENT), filter(nullptr) {}

DeleteStatement::~DeleteStatement() {
  if (filter != nullptr) {
    delete filter;
  }
}

void DeleteStatement::print() const {
  std::cout << "DeleteStatement: FROM " << tableName;
  if (filter != nullptr) {
    std::cout << " WHERE ...";
  }
  std::cout << "\n";
}
