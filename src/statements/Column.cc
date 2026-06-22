#include <statements/Column.h>
#include <iostream>

Column::Column(std::string nameA, ColumnType typeA, bool primaryKeyA, bool notNullA, bool uniqueA)
    : name(std::move(nameA)), type(typeA), primaryKey(primaryKeyA), notNull(notNullA), unique(uniqueA) {}

IntegerColumn::IntegerColumn(std::string nameA, bool primaryKeyA, bool notNullA, bool uniqueA)
    : Column(std::move(nameA), ColumnType::INTEGER, primaryKeyA, notNullA, uniqueA) {}

void IntegerColumn::print() const {
  std::cout << "    " << name << " INTEGER";
  if (primaryKey) std::cout << " PRIMARY KEY";
  if (notNull) std::cout << " NOT NULL";
  if (unique) std::cout << " UNIQUE";
  std::cout << "\n";
}

VarcharColumn::VarcharColumn(std::string nameA, int lengthA, bool primaryKeyA, bool notNullA, bool uniqueA)
    : Column(std::move(nameA), ColumnType::VARCHAR, primaryKeyA, notNullA, uniqueA), varcharLength(lengthA) {}

void VarcharColumn::print() const {
  std::cout << "    " << name << " VARCHAR(" << varcharLength << ")";
  if (primaryKey) std::cout << " PRIMARY KEY";
  if (notNull) std::cout << " NOT NULL";
  if (unique) std::cout << " UNIQUE";
  std::cout << "\n";
}
