#include <parsers/SelectStatementParser.h>
#include <statements/SelectStatement.h>
#include <stdexcept>
#include <string>

SelectStatementParser::SelectStatementParser(std::vector<Token> &tokens)
    : StatementParser(tokens) {}

SelectStatement* SelectStatementParser::parse() {
  // Expect SELECT
  consume(TokenType::SELECT);

  std::vector<std::string> projection = parseProjection();

  consume(TokenType::FROM);

  std::string tableName = parseTableName();

  Expression *filter = nullptr;
  if (accept(TokenType::WHERE)) {
    filter = parseExpr();
  }

  if (m_Pos < m_Tokens.size() && m_Tokens[m_Pos].type == TokenType::END) {
    ++m_Pos;
  }

  return new SelectStatement(std::move(projection), std::move(tableName), filter);
}

std::vector<std::string> SelectStatementParser::parseProjection() {
  std::vector<std::string> result;

  result.push_back(consume(TokenType::IDENTIFIER).value);

  while (accept(TokenType::COMMA)) {
    result.push_back(consume(TokenType::IDENTIFIER).value);
  }

  return result;
}

std::string SelectStatementParser::parseTableName() {
  return consume(TokenType::IDENTIFIER).value;
}

Expression *SelectStatementParser::parseExpr() { return parseOr(); }

Expression *SelectStatementParser::parseOr() {
  Expression *left = parseAnd();

  while (m_Pos < m_Tokens.size() && m_Tokens[m_Pos].type == TokenType::OR) {
    ++m_Pos;
    Expression *right = parseAnd();
    left = new OrExpression(left, right);
  }

  return left;
}

Expression *SelectStatementParser::parseAnd() {
  Expression *left = parseTerm();

  while (m_Pos < m_Tokens.size() && m_Tokens[m_Pos].type == TokenType::AND) {
    ++m_Pos;
    Expression *right = parseTerm();
    left = new AndExpression(left, right);
  }

  return left;
}

Expression *SelectStatementParser::parseTerm() {
  const Token &t = peek();
  if (t.type == TokenType::LPAREN) {
    consume(TokenType::LPAREN);
    Expression *inner = parseExpr();
    consume(TokenType::RPAREN);
    return inner;
  }

  Token leftTok = consume(TokenType::IDENTIFIER);
  Expression *leftExpr = new IdentifierExpression(leftTok.value);

  TokenType op = peek().type;

  if (op == TokenType::EQUALS || op == TokenType::GREATER_THAN ||
      op == TokenType::LESS_THAN) {
    ++m_Pos;

    const Token &rightTok = peek();
    Expression *rightExpr = nullptr;

    if (rightTok.type == TokenType::IDENTIFIER) {
      rightExpr = new IdentifierExpression(consume(TokenType::IDENTIFIER).value);
    } else if (rightTok.type == TokenType::NUMERIC_LITERAL) {
      std::string v = consume(TokenType::NUMERIC_LITERAL).value;
      int iv = 0;
      try {
        iv = std::stoi(v);
      } catch (...) {
        throw std::runtime_error("[SelectParser] Invalid numeric literal");
      }
      rightExpr = new LiteralExpression(iv);
    } else if (rightTok.type == TokenType::STRING_LITERAL) {
      std::string s = consume(TokenType::STRING_LITERAL).value;
      rightExpr = new LiteralExpression(s);
    } else {
      throw std::runtime_error("[SelectParser] Unexpected token in comparison");
    }

    if (op == TokenType::EQUALS) {
      return new EqualityExpression(leftExpr, rightExpr);
    } else if (op == TokenType::GREATER_THAN) {
      return new GreaterExpression(leftExpr, rightExpr);
    } else {
      return new LessExpression(leftExpr, rightExpr);
    }
  }

  throw std::runtime_error("[SelectParser] Expected comparison operator");
}
