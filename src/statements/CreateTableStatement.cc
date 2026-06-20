#include <statements/CreateTableStatement.h>
#include <iostream>

CreateTableStatement::CreateTableStatement(std::string tableNameA)
  : Statement(Statement::StatementType::CREATE_TABLE), tableName(std::move(tableNameA)) {}

void CreateTableStatement::print() const {
  std::cout << "CreateTableStatement: " << tableName << "\n";
}
