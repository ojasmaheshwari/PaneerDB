#ifndef CATALOG_H
#define CATALOG_H

#include "storage/BufferPoolManager.h"
#include "catalog/Schema.h"
#include <unordered_map>
#include <string>

class Catalog {
public:
  Catalog(BufferPoolManager* bpm);
  ~Catalog();

  bool createTable(const std::string& tableName, const Schema& schema);
  Schema* getTable(const std::string& tableName);

  static constexpr page_id_t CATALOG_PAGE_ID = 1;

private:
  BufferPoolManager* bpm;
  std::unordered_map<std::string, Schema*> tables;
};

#endif // CATALOG_H
