#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <string>
#include <variant>
#include <vector>

typedef std::variant<std::string, int, bool> Value;

class Expression {

public:
  virtual ~Expression() = default;
  virtual Value solve() = 0;
};

class LiteralExpression : public Expression {

public:
  LiteralExpression(Value &&valueA);

  Value value;
  Value solve() override;
};

class IdentifierExpression : public Expression {

public:
  // TODO: We also need to pass a record in the constructor
  IdentifierExpression(const std::string &columnNameA);

  std::string columnName;

  Value solve() override;
};

class BinaryExpression : public Expression {

public:
  ~BinaryExpression() override;
  Expression *left;
  Expression *right;

protected:
  BinaryExpression(Expression *leftA, Expression *rightA);
};

class AndExpression : public BinaryExpression {

public:
  AndExpression(Expression *leftA, Expression *rightA);

  Value solve() override;
};

class EqualityExpression : public BinaryExpression {

public:
  EqualityExpression(Expression *leftA, Expression *rightA);

  Value solve() override;
};

class OrExpression : public BinaryExpression {
public:
  OrExpression(Expression *leftA, Expression *rightA);
  Value solve() override;
};

class GreaterExpression : public BinaryExpression {
public:
  GreaterExpression(Expression *leftA, Expression *rightA);
  Value solve() override;
};

class LessExpression : public BinaryExpression {
public:
  LessExpression(Expression *leftA, Expression *rightA);
  Value solve() override;
};

#endif
