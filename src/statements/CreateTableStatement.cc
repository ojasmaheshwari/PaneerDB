#include <statements/CreateTableStatement.h>
#include <iostream>

CreateTableStatement::CreateTableStatement(std::string tableNameA,
                                           std::vector<ColumnDefinition> columnsA)
    : Statement(Statement::StatementType::CREATE_TABLE),
      tableName(std::move(tableNameA)), columns(std::move(columnsA)) {}

void CreateTableStatement::print() const {
  std::cout << "CreateTableStatement: " << tableName << "\n";
  std::cout << "  Columns:\n";

  for (const auto &col : columns) {
    std::cout << "    " << col.name << " ";

    if (col.type == ColumnType::INTEGER) {
      std::cout << "INTEGER";
    } else {
      std::cout << "VARCHAR(" << col.varcharLength << ")";
    }

    if (col.primaryKey) std::cout << " PRIMARY KEY";
    if (col.notNull) std::cout << " NOT NULL";
    if (col.unique) std::cout << " UNIQUE";

    std::cout << "\n";
  }
}