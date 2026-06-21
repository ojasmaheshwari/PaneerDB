#ifndef INSERT_STATEMENT_H
#define INSERT_STATEMENT_H

#include <vector>
#include <string>
#include <statements/Statement.h>

class InsertStatement : public Statement {
public:
  InsertStatement(std::string tableNameA, std::vector<std::string> columnsA,
                  std::vector<std::string> valuesA);

  void print() const override;

  std::string tableName;
  std::vector<std::string> columns; // Empty if query is: INSERT INTO table VALUES (...)
  std::vector<std::string> values;
};

#endif
