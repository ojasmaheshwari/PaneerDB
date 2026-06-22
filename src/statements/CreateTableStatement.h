#ifndef CREATE_TABLE_STATEMENT_H
#define CREATE_TABLE_STATEMENT_H

#include <statements/Statement.h>
#include <string>
#include <vector>

enum class ColumnType { INTEGER, VARCHAR };

struct ColumnDefinition {
  std::string name;
  ColumnType type;
  int varcharLength = 0; // only meaningful for VARCHAR
  bool primaryKey = false;
  bool notNull = false;
  bool unique = false;
};

class CreateTableStatement : public Statement {
public:
  CreateTableStatement(std::string tableNameA, std::vector<ColumnDefinition> columnsA);
  void print() const override;

  std::string tableName;
  std::vector<ColumnDefinition> columns;
};

#endif