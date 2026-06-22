#include <statements/CreateTableStatement.h>
#include <iostream>

CreateTableStatement::CreateTableStatement(std::string tableNameA,
                                           std::vector<Column*> columnsA)
    : Statement(Statement::StatementType::CREATE_TABLE),
      tableName(std::move(tableNameA)), columns(std::move(columnsA)) {}

CreateTableStatement::~CreateTableStatement() {
  for (auto *col : columns) {
    delete col;
  }
}

void CreateTableStatement::print() const {
  std::cout << "CreateTableStatement: " << tableName << "\n";
  std::cout << "  Columns:\n";

  for (const auto *col : columns) {
    col->print();
  }
}