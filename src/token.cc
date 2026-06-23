#include <token.h>

Token::Token(const std::string &valueA, TokenType typeA)
    : value(valueA), type(typeA) {}

std::string Token::getTypeName() const
{
  switch (type)
  {
  case TokenType::SELECT:
    return "SELECT";

  case TokenType::FROM:
    return "FROM";

  case TokenType::WHERE:
    return "WHERE";

  case TokenType::OR:
    return "OR";

  case TokenType::AND:
    return "AND";

  case TokenType::CREATE:
    return "CREATE";

  case TokenType::USE:
    return "USE";

  case TokenType::DATABASE:
    return "DATABASE";

  case TokenType::TABLE:
    return "TABLE";

  case TokenType::INSERT:
    return "INSERT";

  case TokenType::INTO:
    return "INTO";

  case TokenType::VALUES:
    return "VALUES";

  case TokenType::DESC:
    return "DESC";

  case TokenType::STAR:
    return "STAR";

  case TokenType::IDENTIFIER:
    return "IDENTIFIER";

  case TokenType::STRING_LITERAL:
    return "STRING_LITERAL";

  case TokenType::NUMERIC_LITERAL:
    return "NUMERIC_LITERAL";

  case TokenType::EQUALS:
    return "EQUALS";

  case TokenType::GREATER_THAN:
    return "GREATER_THAN";

  case TokenType::LESS_THAN:
    return "LESS_THAN";

  case TokenType::LPAREN:
    return "LPAREN";

  case TokenType::RPAREN:
    return "RPAREN";

  case TokenType::END:
    return "END";

  case TokenType::SHOW:
    return "SHOW";

  case TokenType::TABLES:
    return "TABLES";

  default:
    return "UNKNOWN";
  }
}

std::string Token::getTypeName(TokenType typeA) const
{
  switch (typeA)
  {
  case TokenType::SELECT:
    return "SELECT";

  case TokenType::FROM:
    return "FROM";

  case TokenType::WHERE:
    return "WHERE";

  case TokenType::OR:
    return "OR";

  case TokenType::AND:
    return "AND";

  case TokenType::CREATE:
    return "CREATE";

  case TokenType::TABLE:
    return "TABLE";

  case TokenType::INSERT:
    return "INSERT";

  case TokenType::INTO:
    return "INTO";

  case TokenType::VALUES:
    return "VALUES";

  case TokenType::DESC:
    return "DESC";

  case TokenType::STAR:
    return "STAR";

  case TokenType::USE:
    return "USE";

  case TokenType::DATABASE:
    return "DATABASE";

  case TokenType::INTEGER:
    return "INTEGER";

  case TokenType::VARCHAR:
    return "VARCHAR";

  case TokenType::PRIMARY:
    return "PRIMARY";

  case TokenType::KEY:
    return "KEY";

  case TokenType::NOT:
    return "NOT";

  case TokenType::NULL_KW:
    return "NULL";

  case TokenType::UNIQUE:
    return "UNIQUE";

  case TokenType::IDENTIFIER:
    return "IDENTIFIER";

  case TokenType::STRING_LITERAL:
    return "STRING_LITERAL";

  case TokenType::NUMERIC_LITERAL:
    return "NUMERIC_LITERAL";

  case TokenType::EQUALS:
    return "EQUALS";

  case TokenType::GREATER_THAN:
    return "GREATER_THAN";

  case TokenType::LESS_THAN:
    return "LESS_THAN";

  case TokenType::LPAREN:
    return "LPAREN";

  case TokenType::RPAREN:
    return "RPAREN";

  case TokenType::END:
    return "END";

  case TokenType::SHOW:
    return "SHOW";

  case TokenType::TABLES:
    return "TABLES";

  default:
    return "UNKNOWN";
  }
}
