#include <parsers/ShowTablesStatementParser.h>

ShowTablesStatementParser::ShowTablesStatementParser(std::vector<Token> &tokens) : StatementParser(tokens) {}

ShowTablesStatement* ShowTablesStatementParser::parse() {
  consume(TokenType::SHOW);
  consume(TokenType::TABLES);
  return new ShowTablesStatement();
}
