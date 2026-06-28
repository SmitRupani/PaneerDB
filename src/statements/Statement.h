#ifndef STATEMENT_H
#define STATEMENT_H

class Statement {
public:
  enum class StatementType { SELECT, CREATE_DATABASE, USE_DATABASE, UNKNOWN, CREATE_TABLE, INSERT, SHOW_TABLES, DESCRIBE_TABLE, DELETE_STATEMENT, BEGIN_TRANSACTION, COMMIT_TRANSACTION, ROLLBACK_TRANSACTION };

  explicit Statement(StatementType typeA = StatementType::UNKNOWN) : m_type(typeA) {}
  virtual ~Statement() = default;

  virtual void print() const = 0;

  StatementType getType() const { return m_type; }

protected:
  StatementType m_type;
};

#endif
