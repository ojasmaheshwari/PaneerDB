#include "catalog/Schema.h"
#include <cstring>
#include <utility>
#include <cstdint>

Schema::Schema(std::vector<Column*> columnsA) : columns(std::move(columnsA)) {}

Schema::~Schema() {
  for (auto* col : columns) {
    delete col;
  }
}

Schema::Schema(Schema&& other) noexcept : columns(std::move(other.columns)) {
  other.columns.clear();
}

Schema& Schema::operator=(Schema&& other) noexcept {
  if (this != &other) {
    for (auto* col : columns) {
      delete col;
    }
    columns = std::move(other.columns);
    other.columns.clear();
  }
  return *this;
}

std::string Schema::serialize() const {
  std::string buf;
  uint16_t numColumns = columns.size();
  buf.append(reinterpret_cast<const char*>(&numColumns), sizeof(numColumns));

  for (const auto* col : columns) {
    std::string colBuf = col->serialize();
    buf.append(colBuf);
  }

  return buf;
}

Schema* Schema::deserialize(const std::string& data) {
  size_t offset = 0;
  uint16_t numColumns;
  std::memcpy(&numColumns, data.data() + offset, sizeof(numColumns));
  offset += sizeof(numColumns);

  std::vector<Column*> columns;
  for (uint16_t i = 0; i < numColumns; ++i) {
    Column* col = Column::deserialize(data, offset);
    columns.push_back(col);
  }

  return new Schema(std::move(columns));
}

void Schema::print() const {
  for (const auto* col : columns) {
    col->print();
  }
}
