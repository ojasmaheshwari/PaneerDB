
#ifndef SELECT_STATEMENT_PARSER_H
#define SELECT_STATEMENT_PARSER_H

#include <Expression.h>
#include <token.h>
#include <parsers/StatementParser.h>
#include <statements/SelectStatement.h>
#include <string>
#include <vector>
#include <statements/SelectStatement.h>

class SelectStatementParser : public StatementParser {
public:
	explicit SelectStatementParser(std::vector<Token> &tokens);
	~SelectStatementParser() override = default;

	SelectStatement* parse();

private:
	std::vector<std::string> parseProjection();
	std::string parseTableName();

	// expression parsing
	Expression *parseExpr();
	Expression *parseOr();
	Expression *parseAnd();
	Expression *parseTerm();
};

#endif

