#include <statements/CreateTableStatement.h>
#include <iostream>

CreateTableStatement::CreateTableStatement(std::string tableNameA,
                                           std::map<std::string, Column*> columnsA)
    : Statement(Statement::StatementType::CREATE_TABLE),
      tableName(std::move(tableNameA)), columns(std::move(columnsA)) {}

CreateTableStatement::~CreateTableStatement() {
  for (auto &pair : columns) {
    delete pair.second;
  }
}

void CreateTableStatement::print() const {
  std::cout << "CreateTableStatement: " << tableName << "\n";
  std::cout << "  Columns:\n";

  for (const auto &pair : columns) {
    pair.second->print();
  }
}