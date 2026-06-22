#ifndef SCHEMA_H
#define SCHEMA_H

#include <vector>
#include <string>
#include "statements/Column.h"

class Schema {
public:
  Schema(std::vector<Column*> columnsA);
  ~Schema();

  // Disable copy constructor and assignment to avoid double-free of Column*
  Schema(const Schema&) = delete;
  Schema& operator=(const Schema&) = delete;

  // Allow move
  Schema(Schema&&) noexcept;
  Schema& operator=(Schema&&) noexcept;

  std::string serialize() const;
  static Schema* deserialize(const std::string& data);

  void print() const;

  std::vector<Column*> columns;
};

#endif // SCHEMA_H
