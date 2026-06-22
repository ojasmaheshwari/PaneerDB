#ifndef CREATE_TABLE_STATEMENT_H
#define CREATE_TABLE_STATEMENT_H

#include <statements/Statement.h>
#include <statements/Column.h>
#include <string>
#include <map>

class CreateTableStatement : public Statement {
public:
  CreateTableStatement(std::string tableNameA, std::map<std::string, Column*> columnsA);
  ~CreateTableStatement() override;
  void print() const override;

  std::string tableName;
  std::map<std::string, Column*> columns;
};

#endif