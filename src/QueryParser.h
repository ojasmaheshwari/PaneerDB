#ifndef QUERY_PARSER_H
#define QUERY_PARSER_H

#include <statements/SelectStatement.h>
#include <statements/CreateDatabaseStatement.h>
#include <statements/UseDatabaseStatement.h>
#include <Expression.h>
#include <token.h>
#include <cstddef>
#include <string>
#include <vector>

class QueryParser {

public:
  QueryParser() = default;
  ~QueryParser() = default;

  void tokenize(const std::string &query);

  Statement *parse();

private:
  size_t m_TokenPos;
  std::vector<Token> m_Tokens;
};

#endif
