#include <parsers/CreateTableStatementParser.h>
#include <stdexcept>

CreateTableStatementParser::CreateTableStatementParser(std::vector<Token> &tokens)
    : StatementParser(tokens) {}

CreateTableStatement* CreateTableStatementParser::parse() {
  // Expect CREATE
  consume(TokenType::CREATE);

  // Optionally TABLE keyword
  if (accept(TokenType::TABLE)) {
    // ok
  }

  std::string name = consume(TokenType::IDENTIFIER).value;

  if (m_Pos < m_Tokens.size() && m_Tokens[m_Pos].type == TokenType::END) {
    ++m_Pos;
  }

  return new CreateTableStatement(std::move(name));
}
