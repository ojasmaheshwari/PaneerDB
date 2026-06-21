#ifndef STATEMENT_H
#define STATEMENT_H

class Statement {
public:
  enum class StatementType { SELECT, CREATE_DATABASE, USE_DATABASE, INSERT, UNKNOWN };

  explicit Statement(StatementType typeA = StatementType::UNKNOWN) : m_type(typeA) {}
  virtual ~Statement() = default;

  virtual void print() const = 0;

  StatementType getType() const { return m_type; }

protected:
  StatementType m_type;
};

#endif
