#include "catalog/Catalog.h"
#include "storage/SlottedPage.h"
#include <cstring>
#include <iostream>

Catalog::Catalog(BufferPoolManager *bpm) : bpm(bpm) {
  DiskManager *diskManager = bpm->getDiskManager();

  if (diskManager->getNextPageId() == 0) {
    // Database is empty. Allocate Page 0 (Meta) and Page 1 (Catalog)
    page_id_t metaPageId, catalogPageId;
    Page *metaPage = bpm->newPage(&metaPageId);       // Should be 0
    Page *catalogPage = bpm->newPage(&catalogPageId); // Should be 1

    if (metaPageId != 0 || catalogPageId != 1) {
      std::cerr << "Warning: Allocated pages didn't match expected Meta(0) and "
                   "Catalog(1)\n";
    }

    // Initialize Catalog Page
    SlottedPage sp(catalogPage);
    sp.init();

    bpm->unpinPage(metaPageId, true);
    bpm->unpinPage(catalogPageId, true);
  } else {
    // Load existing catalog from Page 1
    Page *catalogPage = bpm->fetchPage(CATALOG_PAGE_ID);
    if (!catalogPage) {
      std::cerr << "Error: Could not fetch Catalog Page 1\n";
      return;
    }
    SlottedPage sp(catalogPage);

    uint16_t slotCount = sp.getSlotCount();
    for (uint16_t i = 0; i < slotCount; ++i) {
      std::string tupleData = sp.getTuple(i);
      if (tupleData.empty())
        continue; // Lazy deleted

      size_t offset = 0;
      uint16_t nameLen;
      std::memcpy(&nameLen, tupleData.data() + offset, sizeof(nameLen));
      offset += sizeof(nameLen);

      std::string tableName = tupleData.substr(offset, nameLen);
      offset += nameLen;

      std::string schemaData = tupleData.substr(offset);
      Schema *schema = Schema::deserialize(schemaData);

      tables[tableName] = schema;

      std::cout << "[Catalog] Loaded table '" << tableName << "' from disk:\n";
      schema->print();
    }

    bpm->unpinPage(CATALOG_PAGE_ID, false);
  }
}

Catalog::~Catalog() {
  for (auto &pair : tables) {
    delete pair.second;
  }
}

bool Catalog::createTable(const std::string &tableName, const Schema &schema) {
  if (tables.find(tableName) != tables.end()) {
    std::cerr << "Table '" << tableName << "' already exists.\n";
    return false;
  }

  Page *catalogPage = bpm->fetchPage(CATALOG_PAGE_ID);
  if (!catalogPage) {
    std::cerr << "Error: Could not fetch Catalog Page 1\n";
    return false;
  }
  SlottedPage sp(catalogPage);

  // Serialize tuple
  std::string tupleData;
  uint16_t nameLen = tableName.size();
  tupleData.append(reinterpret_cast<const char *>(&nameLen), sizeof(nameLen));
  tupleData.append(tableName);
  tupleData.append(schema.serialize());

  // Insert into SlottedPage
  int32_t slotId = sp.insertTuple(tupleData.data(), tupleData.size());
  if (slotId < 0) {
    std::cerr
        << "Failed to insert table schema into catalog page. Page full?\n";
    bpm->unpinPage(CATALOG_PAGE_ID, false);
    return false;
  }

  bpm->unpinPage(CATALOG_PAGE_ID, true);

  // Add a copy to our in-memory map
  std::string schemaStr = schema.serialize();
  tables[tableName] = Schema::deserialize(schemaStr);

  std::cout << "[Catalog] Created table '" << tableName << "'.\n";
  return true;
}

Schema *Catalog::getTable(const std::string &tableName) {
  if (tables.find(tableName) != tables.end()) {
    return tables[tableName];
  }
  return nullptr;
}
