#ifndef COLUMN_H
#define COLUMN_H

#include <string>

enum class ColumnType { INTEGER, VARCHAR };

class Column {
public:
  Column(std::string nameA, ColumnType typeA, bool primaryKeyA, bool notNullA, bool uniqueA);
  virtual ~Column() = default;

  std::string name;
  ColumnType type;
  bool primaryKey;
  bool notNull;
  bool unique;

  virtual void print() const = 0;
};

class IntegerColumn : public Column {
public:
  IntegerColumn(std::string nameA, bool primaryKeyA, bool notNullA, bool uniqueA);
  void print() const override;
};

class VarcharColumn : public Column {
public:
  VarcharColumn(std::string nameA, int lengthA, bool primaryKeyA, bool notNullA, bool uniqueA);
  void print() const override;

  int varcharLength;
};

#endif
