#ifndef INSERT_STATEMENT_PARSER_H
#define INSERT_STATEMENT_PARSER_H

#include <parsers/StatementParser.h>
#include <statements/InsertStatement.h>
#include <token.h>
#include <string>
#include <vector>

class InsertStatementParser : public StatementParser {
public:
  explicit InsertStatementParser(std::vector<Token> &tokens);
  ~InsertStatementParser() override = default;

  InsertStatement* parse();

private:
  std::string parseTableName();
  std::string parseValue();
};

#endif
