#ifndef CREATE_DATABASE_STATEMENT_H
#define CREATE_DATABASE_STATEMENT_H

#include <statements/Statement.h>
#include <string>

class CreateDatabaseStatement : public Statement {
public:
  CreateDatabaseStatement(std::string dbNameA);
  void print() const override;

  std::string dbName;
};

#endif
