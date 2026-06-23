#include "Engine.h"
#include "DiskManager/DiskManager.h"
#include "catalog/Catalog.h"
#include "statements/CreateDatabaseStatement.h"
#include "statements/CreateTableStatement.h"
#include "statements/DescribeStatement.h"
#include "statements/InsertStatement.h"
#include "statements/SelectStatement.h"
#include "statements/Statement.h"
#include "statements/UseDatabaseStatement.h"
#include "storage/BufferPoolManager.h"
#include "storage/SlottedPage.h"
#include <cstring>
#include <iostream>
#include <stdexcept>

Engine::Engine() : active(false), activeDbName("") {}
Engine::~Engine() = default;

void Engine::useDatabase(const std::string &name) {
  activeDbName = name;
  active = true;

  std::string fileName = name + ".db";
  diskManager = std::make_unique<DiskManager>(fileName);
  bpm = std::make_unique<BufferPoolManager>(10, diskManager.get());
  catalog = std::make_unique<Catalog>(bpm.get());
}

void Engine::createDatabase(const std::string &name) {
  std::string fileName = name + ".db";
  // Instantiating DiskManager will automatically create the file if it doesn't
  // exist
  DiskManager dm(fileName);
  std::cout << "Created database: " << name << std::endl;
}

void Engine::execute(Statement *statement) {
  if (!statement)
    return;

  switch (statement->getType()) {
  case Statement::StatementType::USE_DATABASE: {
    auto useDbStmt = static_cast<UseDatabaseStatement *>(statement);
    useDatabase(useDbStmt->dbName);
    break;
  }
  case Statement::StatementType::CREATE_DATABASE: {
    auto createDbStmt = static_cast<CreateDatabaseStatement *>(statement);
    createDatabase(createDbStmt->dbName);
    break;
  }
  case Statement::StatementType::CREATE_TABLE: {
    if (!active) {
      std::cerr << "Error: No database active. Use 'USE DATABASE' first.\n";
      break;
    }
    auto createTableStmt = static_cast<CreateTableStatement *>(statement);
    Schema schema(std::move(createTableStmt->columns));
    createTableStmt->columns.clear(); // schema takes ownership

    catalog->createTable(createTableStmt->tableName, schema);
    break;
  }
  case Statement::StatementType::SHOW_TABLES: {
    if (!active) {
      std::cerr << "Error: No database active. Use 'USE DATABASE' first.\n";
      break;
    }
    catalog->showTables();
    break;
  }
  case Statement::StatementType::INSERT: {
    if (!active) {
      std::cerr << "Error: No database active. Use 'USE DATABASE' first.\n";
      break;
    }
    auto insertStmt = static_cast<InsertStatement *>(statement);
    executeInsert(insertStmt);
    break;
  }
  case Statement::StatementType::DESCRIBE_TABLE: {
    if (!active) {
      std::cerr << "Error: No database active. Use 'USE DATABASE' first.\n";
      break;
    }
    auto descStmt = static_cast<DescribeStatement *>(statement);
    Schema *schema = catalog->getTable(descStmt->tableName);
    if (schema) {
      schema->print();
    } else {
      std::cerr << "Error: Table '" << descStmt->tableName
                << "' does not exist.\n";
    }
    break;
  }
  case Statement::StatementType::SELECT: {
    if (!active) {
      std::cerr << "Error: No database active. Use 'USE DATABASE' first.\n";
      break;
    }
    auto selectStmt = static_cast<SelectStatement *>(statement);
    executeSelect(selectStmt);
    break;
  }
  default:
    // Other statement types will be handled here later
    break;
  }
}

bool Engine::isDbActive() const { return active; }

std::string Engine::getActiveDatabase() const {
  if (!active) {
    throw std::runtime_error("No active database.");
  }
  return activeDbName;
}

void Engine::executeInsert(InsertStatement *stmt) {
  Schema *schema = catalog->getTable(stmt->tableName);
  if (!schema) {
    std::cerr << "Error: Table '" << stmt->tableName << "' does not exist.\n";
    return;
  }

  // Determine column mapping
  // If stmt->columns is empty, we expect values for all columns in order
  std::vector<Column *> insertColumns;
  if (stmt->columns.empty()) {
    insertColumns = schema->columns;
  } else {
    for (const auto &colName : stmt->columns) {
      bool found = false;
      for (auto *schemaCol : schema->columns) {
        if (schemaCol->name == colName) {
          insertColumns.push_back(schemaCol);
          found = true;
          break;
        }
      }
      if (!found) {
        std::cerr << "Error: Column '" << colName
                  << "' does not exist in table '" << stmt->tableName << "'.\n";
        return;
      }
    }
  }

  if (insertColumns.size() != stmt->values.size()) {
    std::cerr << "Error: Column count does not match value count.\n";
    return;
  }

  std::string tupleData;

  for (size_t i = 0; i < insertColumns.size(); ++i) {
    Column *col = insertColumns[i];
    std::unordered_map<std::string, Value> emptyRow;
    Value val = stmt->values[i]->solve(emptyRow);

    if (col->type == ColumnType::INTEGER) {
      if (val.index() != 1) { // 1 is int
        std::cerr << "Error: Expected integer value for column '" << col->name
                  << "'.\n";
        return;
      }
      int32_t intVal = std::get<int>(val);
      tupleData.append(reinterpret_cast<const char *>(&intVal), sizeof(intVal));
    } else if (col->type == ColumnType::VARCHAR) {
      if (val.index() != 0) { // 0 is std::string
        std::cerr << "Error: Expected string value for column '" << col->name
                  << "'.\n";
        return;
      }
      std::string strVal = std::get<std::string>(val);

      // Need to cast col to VarcharColumn to get varcharLength
      auto *varcharCol = static_cast<VarcharColumn *>(col);
      if (strVal.length() > varcharCol->varcharLength) {
        std::cerr << "Error: Value too long for column '" << col->name
                  << "' (max " << varcharCol->varcharLength << ").\n";
        return;
      }

      uint32_t strLen = strVal.length();
      tupleData.append(reinterpret_cast<const char *>(&strLen), sizeof(strLen));
      tupleData.append(strVal);
    }
  }

  // Insert into data page
  page_id_t currentPageId = schema->firstPageId;
  while (true) {
    Page *page = bpm->fetchPage(currentPageId);
    if (!page) {
      std::cerr << "Error: Could not fetch data page " << currentPageId
                << ".\n";
      return;
    }

    SlottedPage sp(page);
    int32_t slotId = sp.insertTuple(tupleData.data(), tupleData.size());

    if (slotId >= 0) {
      // Successfully inserted
      bpm->unpinPage(currentPageId, true);
      std::cout << "Inserted 1 row into '" << stmt->tableName << "'.\n";
      return;
    }

    // No space on this page
    page_id_t nextPageId = sp.getNextPageId();
    if (nextPageId != -1) {
      // Go to next page
      bpm->unpinPage(currentPageId, false);
      currentPageId = nextPageId;
    } else {
      // Allocate new page
      page_id_t newPageId;
      Page *newPage = bpm->newPage(&newPageId);
      if (!newPage) {
        std::cerr << "Error: Could not allocate new page for table data.\n";
        bpm->unpinPage(currentPageId, false);
        return;
      }

      SlottedPage newSp(newPage);
      newSp.init();

      // Link current page to new page
      sp.setNextPageId(newPageId);
      bpm->unpinPage(currentPageId,
                     true); // Dirty because we updated nextPageId

      // Try inserting into the new page
      slotId = newSp.insertTuple(tupleData.data(), tupleData.size());
      if (slotId < 0) {
        std::cerr << "Error: Tuple is too large to fit in an empty page.\n";
        bpm->unpinPage(newPageId, false);
        return;
      }

      bpm->unpinPage(newPageId, true);
      std::cout << "Inserted 1 row into '" << stmt->tableName
                << "' (allocated new page " << newPageId << ").\n";
      return;
    }
  }
}

void Engine::executeSelect(SelectStatement *stmt) {
  Schema *schema = catalog->getTable(stmt->tableName);
  if (!schema) {
    std::cerr << "Error: Table '" << stmt->tableName << "' does not exist.\n";
    return;
  }

  // Resolve projection
  std::vector<Column *> selectColumns;
  if (stmt->projection.size() == 1 && stmt->projection[0] == "*") {
    selectColumns = schema->columns;
  } else {
    for (const auto &colName : stmt->projection) {
      bool found = false;
      for (auto *schemaCol : schema->columns) {
        if (schemaCol->name == colName) {
          selectColumns.push_back(schemaCol);
          found = true;
          break;
        }
      }
      if (!found) {
        std::cerr << "Error: Column '" << colName
                  << "' does not exist in table '" << stmt->tableName << "'.\n";
        return;
      }
    }
  }

  std::vector<std::vector<std::string>> results;
  std::vector<std::string> headers;
  for (auto *col : selectColumns)
    headers.push_back(col->name);
  results.push_back(headers);

  std::vector<size_t> colWidths(selectColumns.size(), 0);
  for (size_t i = 0; i < headers.size(); ++i) {
    colWidths[i] = headers[i].length();
  }

  // Scan pages
  page_id_t currentPageId = schema->firstPageId;
  while (currentPageId != -1) {
    Page *page = bpm->fetchPage(currentPageId);
    if (!page) {
      std::cerr << "Error: Could not fetch data page " << currentPageId
                << ".\n";
      return;
    }

    SlottedPage sp(page);
    uint16_t slotCount = sp.getSlotCount();

    for (slot_id_t i = 0; i < slotCount; ++i) {
      std::string tupleData = sp.getTuple(i);
      if (tupleData.empty())
        continue; // lazy deleted

      std::unordered_map<std::string, Value> row;
      size_t offset = 0;

      for (auto *col : schema->columns) {
        if (col->type == ColumnType::INTEGER) {
          int32_t intVal;
          std::memcpy(&intVal, tupleData.data() + offset, sizeof(int32_t));
          offset += sizeof(int32_t);
          row[col->name] = intVal;
        } else if (col->type == ColumnType::VARCHAR) {
          uint32_t strLen;
          std::memcpy(&strLen, tupleData.data() + offset, sizeof(uint32_t));
          offset += sizeof(uint32_t);
          std::string strVal(tupleData.data() + offset, strLen);
          offset += strLen;
          row[col->name] = strVal;
        }
      }

      bool matches = true;
      if (stmt->filter != nullptr) {
        try {
          Value filterVal = stmt->filter->solve(row);
          matches = std::visit(
              [](const auto &v) -> bool {
                using T = std::decay_t<decltype(v)>;
                if constexpr (std::is_same_v<T, bool>)
                  return v;
                else if constexpr (std::is_same_v<T, std::string>)
                  return !v.empty();
                else if constexpr (std::is_same_v<T, int>)
                  return v > 0;
              },
              filterVal);
        } catch (const std::exception &e) {
          std::cerr << "Error evaluating filter: " << e.what() << "\n";
          bpm->unpinPage(page->getPageId(), false);
          return;
        }
      }

      if (matches) {
        std::vector<std::string> rowStr;
        for (size_t c = 0; c < selectColumns.size(); ++c) {
          Column *col = selectColumns[c];
          std::string valStr;
          if (row[col->name].index() == 1) { // int
            valStr = std::to_string(std::get<int>(row[col->name]));
          } else { // string
            valStr = std::get<std::string>(row[col->name]);
          }
          rowStr.push_back(valStr);
          if (valStr.length() > colWidths[c]) {
            colWidths[c] = valStr.length();
          }
        }
        results.push_back(rowStr);
      }
    }

    currentPageId = sp.getNextPageId();
    bpm->unpinPage(page->getPageId(), false);
  }

  auto printBorder = [&]() {
    std::cout << "+";
    for (size_t w : colWidths) {
      for (size_t i = 0; i < w + 2; ++i)
        std::cout << "-";
      std::cout << "+";
    }
    std::cout << "\n";
  };

  printBorder();
  std::cout << "|";
  for (size_t i = 0; i < headers.size(); ++i) {
    std::cout << " " << headers[i];
    for (size_t p = 0; p < colWidths[i] - headers[i].length(); ++p)
      std::cout << " ";
    std::cout << " |";
  }
  std::cout << "\n";
  printBorder();

  for (size_t r = 1; r < results.size(); ++r) {
    std::cout << "|";
    for (size_t i = 0; i < selectColumns.size(); ++i) {
      std::string val = results[r][i];
      std::cout << " " << val;
      for (size_t p = 0; p < colWidths[i] - val.length(); ++p)
        std::cout << " ";
      std::cout << " |";
    }
    std::cout << "\n";
  }

  if (results.size() > 1) {
    printBorder();
  }

  std::cout << (results.size() - 1) << " rows in set.\n";
}
