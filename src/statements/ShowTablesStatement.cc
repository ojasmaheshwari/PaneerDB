#include "statements/ShowTablesStatement.h"

ShowTablesStatement::ShowTablesStatement() : Statement(Statement::StatementType::SHOW_TABLES) {}

ShowTablesStatement::~ShowTablesStatement() = default;

void ShowTablesStatement::print() const {
  // Empty
}
