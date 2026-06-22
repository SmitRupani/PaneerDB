#ifndef INSERT_STATEMENT_H
#define INSERT_STATEMENT_H

#include <statements/Statement.h>
#include <Expression.h>
#include <string>
#include <vector>

class InsertStatement : public Statement {
public:
  InsertStatement(std::string tableNameA, std::vector<std::string> columnsA, std::vector<Expression*> valuesA);
  ~InsertStatement() override;

  void print() const override;

  std::string tableName;
  std::vector<std::string> columns;
  std::vector<Expression*> values;
};

#endif
