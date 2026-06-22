#include <statements/Column.h>
#include <iostream>
#include <cstring>
#include <vector>
#include <cstdint>

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

std::string IntegerColumn::serialize() const {
  std::string buf;
  uint16_t nameLen = name.size();
  buf.append(reinterpret_cast<const char*>(&nameLen), sizeof(nameLen));
  buf.append(name);
  
  uint8_t t = static_cast<uint8_t>(ColumnType::INTEGER);
  buf.append(reinterpret_cast<const char*>(&t), sizeof(t));
  
  uint8_t pk = primaryKey ? 1 : 0;
  uint8_t nn = notNull ? 1 : 0;
  uint8_t uq = unique ? 1 : 0;
  buf.append(reinterpret_cast<const char*>(&pk), sizeof(pk));
  buf.append(reinterpret_cast<const char*>(&nn), sizeof(nn));
  buf.append(reinterpret_cast<const char*>(&uq), sizeof(uq));
  
  return buf;
}

std::string VarcharColumn::serialize() const {
  std::string buf;
  uint16_t nameLen = name.size();
  buf.append(reinterpret_cast<const char*>(&nameLen), sizeof(nameLen));
  buf.append(name);
  
  uint8_t t = static_cast<uint8_t>(ColumnType::VARCHAR);
  buf.append(reinterpret_cast<const char*>(&t), sizeof(t));
  
  uint8_t pk = primaryKey ? 1 : 0;
  uint8_t nn = notNull ? 1 : 0;
  uint8_t uq = unique ? 1 : 0;
  buf.append(reinterpret_cast<const char*>(&pk), sizeof(pk));
  buf.append(reinterpret_cast<const char*>(&nn), sizeof(nn));
  buf.append(reinterpret_cast<const char*>(&uq), sizeof(uq));
  
  uint32_t vlen = varcharLength;
  buf.append(reinterpret_cast<const char*>(&vlen), sizeof(vlen));
  
  return buf;
}

Column* Column::deserialize(const std::string& data, size_t& offset) {
  uint16_t nameLen;
  std::memcpy(&nameLen, data.data() + offset, sizeof(nameLen));
  offset += sizeof(nameLen);
  
  std::string name = data.substr(offset, nameLen);
  offset += nameLen;
  
  uint8_t t;
  std::memcpy(&t, data.data() + offset, sizeof(t));
  offset += sizeof(t);
  
  uint8_t pk, nn, uq;
  std::memcpy(&pk, data.data() + offset, sizeof(pk)); offset += sizeof(pk);
  std::memcpy(&nn, data.data() + offset, sizeof(nn)); offset += sizeof(nn);
  std::memcpy(&uq, data.data() + offset, sizeof(uq)); offset += sizeof(uq);
  
  if (static_cast<ColumnType>(t) == ColumnType::INTEGER) {
    return new IntegerColumn(name, pk == 1, nn == 1, uq == 1);
  } else {
    uint32_t vlen;
    std::memcpy(&vlen, data.data() + offset, sizeof(vlen));
    offset += sizeof(vlen);
    return new VarcharColumn(name, vlen, pk == 1, nn == 1, uq == 1);
  }
}
