#include <parsers/InsertStatementParser.h>
#include <stdexcept>
#include <string>

InsertStatementParser::InsertStatementParser(std::vector<Token> &tokens)
    : StatementParser(tokens) {}

InsertStatement* InsertStatementParser::parse() {
  // Expect INSERT INTO
  consume(TokenType::INSERT);
  consume(TokenType::INTO);

  std::string tableName = parseTableName();

  std::vector<std::string> columns;

  // Optional explicit column list: INSERT INTO table (col1, col2, ...) VALUES (...)
  if (accept(TokenType::LPAREN)) {
    columns.push_back(consume(TokenType::IDENTIFIER).value);
    while (accept(TokenType::COMMA)) {
      columns.push_back(consume(TokenType::IDENTIFIER).value);
    }
    consume(TokenType::RPAREN);
  }

  consume(TokenType::VALUES);
  consume(TokenType::LPAREN);

  std::vector<std::string> values;
  values.push_back(parseValue());
  while (accept(TokenType::COMMA)) {
    values.push_back(parseValue());
  }
  consume(TokenType::RPAREN);

  if (m_Pos < m_Tokens.size() && m_Tokens[m_Pos].type == TokenType::END) {
    ++m_Pos;
  }

  return new InsertStatement(std::move(tableName), std::move(columns),
                             std::move(values));
}

std::string InsertStatementParser::parseTableName() {
  return consume(TokenType::IDENTIFIER).value;
}

std::string InsertStatementParser::parseValue() {
  const Token &t = peek();
  if (t.type == TokenType::STRING_LITERAL ||
      t.type == TokenType::NUMERIC_LITERAL) {
    ++m_Pos;
    return t.value;
  }
  throw std::runtime_error(
      "[InsertParser] Expected literal value in VALUES clause");
}
