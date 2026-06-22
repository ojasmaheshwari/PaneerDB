#include <parsers/CreateTableStatementParser.h>
#include <stdexcept>

CreateTableStatementParser::CreateTableStatementParser(std::vector<Token> &tokens)
    : StatementParser(tokens) {}

CreateTableStatement* CreateTableStatementParser::parse() {
  consume(TokenType::CREATE);
  consume(TokenType::TABLE);

  std::string name = consume(TokenType::IDENTIFIER).value;

  consume(TokenType::LPAREN);

  std::vector<ColumnDefinition> columns;
  columns.push_back(parseColumnDefinition());

  while (accept(TokenType::COMMA)) {
    // allow a trailing comma before the closing paren
    if (peek().type == TokenType::RPAREN) {
      break;
    }
    columns.push_back(parseColumnDefinition());
  }

  consume(TokenType::RPAREN);

  if (m_Pos < m_Tokens.size() && m_Tokens[m_Pos].type == TokenType::END) {
    ++m_Pos;
  }

  return new CreateTableStatement(std::move(name), std::move(columns));
}

ColumnDefinition CreateTableStatementParser::parseColumnDefinition() {
  ColumnDefinition col;
  col.name = consume(TokenType::IDENTIFIER).value;

  if (accept(TokenType::INTEGER)) {
    col.type = ColumnType::INTEGER;
  } else if (accept(TokenType::VARCHAR)) {
    col.type = ColumnType::VARCHAR;

    consume(TokenType::LPAREN);
    std::string lenStr = consume(TokenType::NUMERIC_LITERAL).value;

    try {
      col.varcharLength = std::stoi(lenStr);
    } catch (...) {
      throw std::runtime_error("[CreateTableParser] Invalid VARCHAR length");
    }

    consume(TokenType::RPAREN);
  } else {
    throw std::runtime_error(
        "[CreateTableParser] Expected data type (INTEGER or VARCHAR) for column '" +
        col.name + "'");
  }

  // Column constraints, in any order, any combination
  while (true) {
    if (accept(TokenType::PRIMARY)) {
      consume(TokenType::KEY);
      col.primaryKey = true;
    } else if (accept(TokenType::NOT)) {
      consume(TokenType::NULL_KW);
      col.notNull = true;
    } else if (accept(TokenType::UNIQUE)) {
      col.unique = true;
    } else {
      break;
    }
  }

  return col;
}