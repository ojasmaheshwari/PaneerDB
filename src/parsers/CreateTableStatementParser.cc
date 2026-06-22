#include <parsers/CreateTableStatementParser.h>
#include <stdexcept>

CreateTableStatementParser::CreateTableStatementParser(
    std::vector<Token> &tokens)
    : StatementParser(tokens) {}

CreateTableStatement *CreateTableStatementParser::parse() {
  consume(TokenType::CREATE);
  consume(TokenType::TABLE);

  std::string name = consume(TokenType::IDENTIFIER).value;

  consume(TokenType::LPAREN);

  std::map<std::string, Column *> columns;
  Column *firstCol = parseColumnDefinition();
  columns[firstCol->name] = firstCol;

  while (accept(TokenType::COMMA)) {
    // allow a trailing comma before the closing paren
    if (peek().type == TokenType::RPAREN) {
      break;
    }
    Column *col = parseColumnDefinition();
    columns[col->name] = col;
  }

  consume(TokenType::RPAREN);

  if (m_Pos < m_Tokens.size() && m_Tokens[m_Pos].type == TokenType::END) {
    ++m_Pos;
  }

  return new CreateTableStatement(std::move(name), std::move(columns));
}

Column *CreateTableStatementParser::parseColumnDefinition() {
  std::string colName = consume(TokenType::IDENTIFIER).value;
  ColumnType type;
  int varcharLength = 0;

  if (accept(TokenType::INTEGER)) {
    type = ColumnType::INTEGER;
  } else if (accept(TokenType::VARCHAR)) {
    type = ColumnType::VARCHAR;

    consume(TokenType::LPAREN);
    std::string lenStr = consume(TokenType::NUMERIC_LITERAL).value;

    try {
      varcharLength = std::stoi(lenStr);
    } catch (...) {
      throw std::runtime_error("[CreateTableParser] Invalid VARCHAR length");
    }

    consume(TokenType::RPAREN);
  } else {
    throw std::runtime_error("[CreateTableParser] Expected data type (INTEGER "
                             "or VARCHAR) for column '" +
                             colName + "'");
  }

  bool pk = false, nn = false, uq = false;

  // Column constraints, in any order, any combination
  while (true) {
    if (accept(TokenType::PRIMARY)) {
      consume(TokenType::KEY);
      pk = true;
    } else if (accept(TokenType::NOT)) {
      consume(TokenType::NULL_KW);
      nn = true;
    } else if (accept(TokenType::UNIQUE)) {
      uq = true;
    } else {
      break;
    }
  }

  if (type == ColumnType::INTEGER) {
    return new IntegerColumn(colName, pk, nn, uq);
  } else {
    return new VarcharColumn(colName, varcharLength, pk, nn, uq);
  }
}