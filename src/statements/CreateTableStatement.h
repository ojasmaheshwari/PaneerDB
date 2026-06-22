#ifndef CREATE_TABLE_STATEMENT_H
#define CREATE_TABLE_STATEMENT_H

#include <statements/Statement.h>
#include <statements/Column.h>
#include <string>
#include <vector>

class CreateTableStatement : public Statement {
public:
  CreateTableStatement(std::string tableNameA, std::vector<Column*> columnsA);
  ~CreateTableStatement() override;
  void print() const override;

  std::string tableName;
  std::vector<Column*> columns;
};

#endif