#include "statements/InsertStatement.h"
#include <iostream>

InsertStatement::InsertStatement(std::string tableNameA,
                                 std::vector<std::string> columnsA,
                                 std::vector<std::string> valuesA)
    : Statement(Statement::StatementType::INSERT),
      tableName(std::move(tableNameA)), columns(std::move(columnsA)),
      values(std::move(valuesA)) {}

void InsertStatement::print() const {
  std::cout << "InsertStatement:\n";
  std::cout << "  Table: " << tableName << "\n";
  std::cout << "  Columns: ";
  for (size_t i = 0; i < columns.size(); ++i) {
    std::cout << columns[i];
    if (i + 1 < columns.size())
      std::cout << ", ";
  }
  std::cout << "\n";
  std::cout << "  Values: ";
  for (size_t i = 0; i < values.size(); ++i) {
    std::cout << values[i];
    if (i + 1 < values.size())
      std::cout << ", ";
  }
  std::cout << "\n";
}
