#include "statements/SelectStatement.h"
#include <iostream>

SelectStatement::SelectStatement(std::vector<std::string> projectionA,
                                 std::string tableNameA, Expression *filterA)
    : Statement(Statement::StatementType::SELECT),
      projection(std::move(projectionA)), tableName(std::move(tableNameA)),
      filter(filterA) {}

void SelectStatement::print() const {
  std::cout << "SelectStatement:\n";
  std::cout << "  Table: " << tableName << "\n";
  std::cout << "  Projection: ";
  for (size_t i = 0; i < projection.size(); ++i) {
    std::cout << projection[i];
    if (i + 1 < projection.size())
      std::cout << ", ";
  }
  std::cout << "\n";
}
