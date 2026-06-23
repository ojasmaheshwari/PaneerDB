#ifndef SHOW_TABLES_STATEMENT_H
#define SHOW_TABLES_STATEMENT_H

#include "statements/Statement.h"
#include <iostream>

class ShowTablesStatement : public Statement {
public:
  ShowTablesStatement();
  ~ShowTablesStatement() override;

  void print() const override;
};

#endif // SHOW_TABLES_STATEMENT_H
