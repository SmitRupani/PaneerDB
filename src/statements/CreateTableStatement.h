#ifndef CREATE_TABLE_STATEMENT_H
#define CREATE_TABLE_STATEMENT_H

#include <statements/Statement.h>
#include <string>

class CreateTableStatement : public Statement {
public:
  CreateTableStatement(std::string tableNameA);
  void print() const override;

  std::string tableName;
};

#endif
