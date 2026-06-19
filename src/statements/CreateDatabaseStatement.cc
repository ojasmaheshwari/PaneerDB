#include <statements/CreateDatabaseStatement.h>
#include <iostream>

CreateDatabaseStatement::CreateDatabaseStatement(std::string dbNameA)
  : Statement(Statement::StatementType::CREATE_DATABASE), dbName(std::move(dbNameA)) {}

void CreateDatabaseStatement::print() const {
  std::cout << "CreateDatabaseStatement: " << dbName << "\n";
}
