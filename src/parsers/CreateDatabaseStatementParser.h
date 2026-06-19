#ifndef CREATE_DATABASE_STATEMENT_PARSER_H
#define CREATE_DATABASE_STATEMENT_PARSER_H

#include <parsers/StatementParser.h>
#include <statements/CreateDatabaseStatement.h>
#include <token.h>
#include <string>
#include <vector>

class CreateDatabaseStatementParser : public StatementParser {
public:
  explicit CreateDatabaseStatementParser(std::vector<Token> &tokens);
  ~CreateDatabaseStatementParser() override = default;

  CreateDatabaseStatement* parse();
};

#endif
