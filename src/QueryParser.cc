#include <Expression.h>
#include <QueryParser.h>
#include <cassert>
#include <cctype>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <parsers/CreateDatabaseStatementParser.h>
#include <parsers/CreateTableStatementParser.h>
#include <parsers/InsertStatementParser.h>
#include <parsers/SelectStatementParser.h>
#include <parsers/UseDatabaseStatementParser.h>
#include <stdexcept>
#include <string>
#include <token.h>
#include <vector>
#include "statements/ShowTablesStatement.h"
#include "parsers/ShowTablesStatementParser.h"
#include "parsers/DescribeStatementParser.h"

void QueryParser::tokenize(const std::string &query) {
  using std::isspace, std::isalpha, std::isdigit;

  size_t i = 0;

  auto getWord = [&query, &i]() -> std::string {
    std::string word;
    while (i < query.size()) {
      if (isspace(query[i]) || query[i] == '<' || query[i] == '>' ||
          query[i] == '=' || query[i] == '(' || query[i] == ')' ||
          query[i] == ';' || query[i] == ',') {
        break;
      }

      word += query[i];
      ++i;
    }
    return word;
  };

  auto isNumber = [](const std::string &word) -> char * {
    char *num;
    strtol(word.c_str(), &num, 10);

    return num;
  };

  while (i < query.size()) {
    auto c = static_cast<unsigned char>(query[i]);

    if (isalpha(c) || c == '_') {

      // Word starts with an alphabet or _
      // Likely a SQL keyword | string literal | IDENTIFIER

      std::string word = getWord();
      std::string upperWord = word;
      for (char &ch : upperWord) {
        ch = std::toupper(static_cast<unsigned char>(ch));
      }

      if (upperWord == "SELECT") {
        m_Tokens.emplace_back("SELECT", TokenType::SELECT);
      } else if (upperWord == "FROM") {
        m_Tokens.emplace_back("FROM", TokenType::FROM);
      } else if (upperWord == "WHERE") {
        m_Tokens.emplace_back("WHERE", TokenType::WHERE);
      } else if (upperWord == "OR") {
        m_Tokens.emplace_back("OR", TokenType::OR);
      } else if (upperWord == "AND") {
        m_Tokens.emplace_back("AND", TokenType::AND);
      } else if (upperWord == "CREATE") {
        m_Tokens.emplace_back("CREATE", TokenType::CREATE);
      } else if (upperWord == "USE") {
        m_Tokens.emplace_back("USE", TokenType::USE);
      } else if (upperWord == "DATABASE") {
        m_Tokens.emplace_back("DATABASE", TokenType::DATABASE);
      } else if (upperWord == "SHOW") {
        m_Tokens.emplace_back("SHOW", TokenType::SHOW);
      } else if (upperWord == "TABLES") {
        m_Tokens.emplace_back("TABLES", TokenType::TABLES);
      } else if (upperWord == "TABLE") {
        m_Tokens.emplace_back("TABLE", TokenType::TABLE);
      } else if (upperWord == "INSERT") {
        m_Tokens.emplace_back("INSERT", TokenType::INSERT);
      } else if (upperWord == "INTO") {
        m_Tokens.emplace_back("INTO", TokenType::INTO);
      } else if (upperWord == "VALUES") {
        m_Tokens.emplace_back("VALUES", TokenType::VALUES);
      } else if (upperWord == "DESC" || upperWord == "DESCRIBE") {
        m_Tokens.emplace_back("DESC", TokenType::DESC);
      } else if (upperWord == "INTEGER" || upperWord == "INT") {
        m_Tokens.emplace_back("INTEGER", TokenType::INTEGER);
      } else if (upperWord == "VARCHAR") {
        m_Tokens.emplace_back("VARCHAR", TokenType::VARCHAR);
      } else if (upperWord == "PRIMARY") {
        m_Tokens.emplace_back("PRIMARY", TokenType::PRIMARY);
      } else if (upperWord == "KEY") {
        m_Tokens.emplace_back("KEY", TokenType::KEY);
      } else if (upperWord == "NOT") {
        m_Tokens.emplace_back("NOT", TokenType::NOT);
      } else if (upperWord == "NULL") {
        m_Tokens.emplace_back("NULL", TokenType::NULL_KW);
      } else if (upperWord == "UNIQUE") {
        m_Tokens.emplace_back("UNIQUE", TokenType::UNIQUE);
      } else {
        m_Tokens.emplace_back(word, TokenType::IDENTIFIER);
      }
    } else if (c == '\"') {
      std::string word = getWord();

      if (word.size() == 1 || word.back() != '\"') {
        throw std::runtime_error(
            "[tokenizer] Missing end quotation near position " +
            std::to_string(i));
      }

      std::string literal = word.substr(1, word.size() - 2);
      m_Tokens.emplace_back(literal, TokenType::STRING_LITERAL);
    } else if (isdigit(c)) {
      std::string word = getWord();
      m_Tokens.emplace_back(word, TokenType::NUMERIC_LITERAL);
    } else if (isspace(c)) {
      ++i;
    } else {
      if (c == '<') {
        m_Tokens.emplace_back("<", TokenType::LESS_THAN);
        ++i;
      } else if (c == '>') {
        m_Tokens.emplace_back(">", TokenType::GREATER_THAN);
        ++i;
      } else if (c == '=') {
        m_Tokens.emplace_back("=", TokenType::EQUALS);
        ++i;
      } else if (c == '(') {
        m_Tokens.emplace_back("(", TokenType::LPAREN);
        ++i;
      } else if (c == ')') {
        m_Tokens.emplace_back(")", TokenType::RPAREN);
        ++i;
      } else if (c == ',') {
        m_Tokens.emplace_back(",", TokenType::COMMA);
        ++i;
      } else if (c == ';') {
        m_Tokens.emplace_back(";", TokenType::END);
        break;
      } else if (c == '*') {
        m_Tokens.emplace_back("*", TokenType::STAR);
        ++i;
      } else {
        std::string error = "[tokenizer] Unrecognized symbol (" +
                            std::string(1, c) + ") CODE " +
                            std::to_string((int)c) + " near position " +
                            std::to_string(i);
        throw std::runtime_error(error);
      }
    }
  }

  if (m_Tokens.empty()) {
    throw std::runtime_error("[tokenizer] Empty query");
  }

  if (m_Tokens.back().type != TokenType::END) {
    throw std::runtime_error("[tokenizer] No semicolon at end of query");
  }
}

Statement *QueryParser::parse() {
  if (m_Tokens.empty()) {
    throw new std::runtime_error("[Parser] Nothing to parse");
  }

  Token &firstToken = m_Tokens[0];

  if (m_Tokens[0].type == TokenType::SELECT) {
    SelectStatementParser parser(m_Tokens);
    return parser.parse();
  }

  if (m_Tokens[0].type == TokenType::CREATE) {
    if (m_Tokens.size() > 1 && m_Tokens[1].type == TokenType::DATABASE) {
      CreateDatabaseStatementParser parser(m_Tokens);
      return parser.parse();
    } else if (m_Tokens.size() > 1 && m_Tokens[1].type == TokenType::TABLE) {
      CreateTableStatementParser parser(m_Tokens);
      return parser.parse();
    } else {
      throw std::runtime_error("[Parser] Invalid CREATE statement - expected "
                               "DATABASE or TABLE keyword");
    }
  }

  if (m_Tokens[0].type == TokenType::USE) {
    UseDatabaseStatementParser parser(m_Tokens);
    return parser.parse();
  }

  if (m_Tokens[0].type == TokenType::INSERT) {
    InsertStatementParser parser(m_Tokens);
    return parser.parse();
  }

  if (m_Tokens[0].type == TokenType::SHOW) {
    ShowTablesStatementParser parser(m_Tokens);
    return parser.parse();
  }

  if (m_Tokens[0].type == TokenType::DESC) {
    DescribeStatementParser parser(m_Tokens);
    return parser.parse();
  }

  throw std::runtime_error("[Parser] Invalid or unsupported query type - " +
                           firstToken.value);
}
