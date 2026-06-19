#ifndef USE_DATABASE_STATEMENT_H
#define USE_DATABASE_STATEMENT_H

#include <statements/Statement.h>
#include <string>

class UseDatabaseStatement : public Statement {
public:
  UseDatabaseStatement(std::string dbNameA);
  void print() const override;

  std::string dbName;
};

#endif
