#ifndef SHOW_TABLES_STATEMENT_PARSER_H
#define SHOW_TABLES_STATEMENT_PARSER_H

#include <parsers/StatementParser.h>
#include <statements/ShowTablesStatement.h>

class ShowTablesStatementParser : public StatementParser {
public:
  explicit ShowTablesStatementParser(std::vector<Token> &tokens);
  ~ShowTablesStatementParser() override = default;

  ShowTablesStatement* parse();
};

#endif // SHOW_TABLES_STATEMENT_PARSER_H
