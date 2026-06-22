#ifndef CREATE_TABLE_STATEMENT_PARSER_H
#define CREATE_TABLE_STATEMENT_PARSER_H

#include <parsers/StatementParser.h>
#include <statements/CreateTableStatement.h>
#include <token.h>
#include <string>
#include <vector>

class CreateTableStatementParser : public StatementParser {
public:
  explicit CreateTableStatementParser(std::vector<Token> &tokens);
  ~CreateTableStatementParser() override = default;

  CreateTableStatement* parse();

private:
  ColumnDefinition parseColumnDefinition();
};

#endif