#include <parsers/UseDatabaseStatementParser.h>
#include <stdexcept>

UseDatabaseStatementParser::UseDatabaseStatementParser(std::vector<Token> &tokens)
    : StatementParser(tokens) {}

UseDatabaseStatement* UseDatabaseStatementParser::parse() {
  // Expect USE
  consume(TokenType::USE);

  // Optional DATABASE
  if (accept(TokenType::DATABASE)) {
    // continue
  }

  std::string name = consume(TokenType::IDENTIFIER).value;

  if (m_Pos < m_Tokens.size() && m_Tokens[m_Pos].type == TokenType::END) {
    ++m_Pos;
  }

  return new UseDatabaseStatement(std::move(name));
}
