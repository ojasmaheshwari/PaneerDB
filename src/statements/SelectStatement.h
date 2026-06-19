#ifndef SELECT_STATEMENT_H
#define SELECT_STATEMENT_H

#include <vector>
#include <string>
#include <statements/Statement.h>
#include <Expression.h>

class SelectStatement : public Statement {
public:
  SelectStatement(std::vector<std::string> projectionA,
                  std::string tableNameA, Expression *filterA);

  void print() const override;

  std::vector<std::string> projection;
  std::string tableName;
  Expression *filter;
};

#endif