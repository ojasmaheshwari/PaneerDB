#include <parsers/CreateDatabaseStatementParser.h>
#include <stdexcept>

CreateDatabaseStatementParser::CreateDatabaseStatementParser(std::vector<Token> &tokens)
    : StatementParser(tokens) {}

CreateDatabaseStatement* CreateDatabaseStatementParser::parse() {
  // Expect CREATE
  consume(TokenType::CREATE);

  // Optionally DATABASE keyword
  if (accept(TokenType::DATABASE)) {
    // ok
  }

  std::string name = consume(TokenType::IDENTIFIER).value;

  if (m_Pos < m_Tokens.size() && m_Tokens[m_Pos].type == TokenType::END) {
    ++m_Pos;
  }

  return new CreateDatabaseStatement(std::move(name));
}
