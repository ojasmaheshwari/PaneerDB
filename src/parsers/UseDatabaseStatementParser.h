#ifndef USE_DATABASE_STATEMENT_PARSER_H
#define USE_DATABASE_STATEMENT_PARSER_H

#include <parsers/StatementParser.h>
#include <statements/UseDatabaseStatement.h>
#include <token.h>
#include <string>
#include <vector>

class UseDatabaseStatementParser : public StatementParser {
public:
  explicit UseDatabaseStatementParser(std::vector<Token> &tokens);
  ~UseDatabaseStatementParser() override = default;

  UseDatabaseStatement* parse();
};

#endif
