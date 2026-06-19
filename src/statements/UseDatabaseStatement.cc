#include <statements/UseDatabaseStatement.h>
#include <iostream>

UseDatabaseStatement::UseDatabaseStatement(std::string dbNameA)
  : Statement(Statement::StatementType::USE_DATABASE), dbName(std::move(dbNameA)) {}

void UseDatabaseStatement::print() const {
  std::cout << "UseDatabaseStatement: " << dbName << "\n";
}
