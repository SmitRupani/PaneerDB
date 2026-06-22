#include <statements/InsertStatement.h>
#include <iostream>

InsertStatement::InsertStatement(std::string tableNameA, std::vector<std::string> columnsA, std::vector<Expression*> valuesA)
    : Statement(Statement::StatementType::INSERT), tableName(std::move(tableNameA)), columns(std::move(columnsA)), values(std::move(valuesA)) {}

InsertStatement::~InsertStatement() {
  for (auto *expr : values) {
    delete expr;
  }
}

void InsertStatement::print() const {
  std::cout << "InsertStatement: " << tableName << "\n";
  if (!columns.empty()) {
    std::cout << "  Columns: ";
    for (size_t i = 0; i < columns.size(); ++i) {
      std::cout << columns[i];
      if (i < columns.size() - 1) std::cout << ", ";
    }
    std::cout << "\n";
  }
  std::cout << "  Values: ";
  for (size_t i = 0; i < values.size(); ++i) {
    std::unordered_map<std::string, Value> emptyRow;
    auto val = values[i]->solve(emptyRow);
    std::visit([](const auto &v) {
      using T = std::decay_t<decltype(v)>;
      if constexpr (std::is_same_v<T, std::string>) {
        std::cout << "\"" << v << "\"";
      } else if constexpr (std::is_same_v<T, int>) {
        std::cout << v;
      } else if constexpr (std::is_same_v<T, bool>) {
        std::cout << (v ? "TRUE" : "FALSE");
      }
    }, val);
    
    if (i < values.size() - 1) std::cout << ", ";
  }
  std::cout << "\n";
}
