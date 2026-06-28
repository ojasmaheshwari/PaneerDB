#include "Engine.h"
#include "DiskManager/DiskManager.h"
#include "catalog/Catalog.h"
#include "statements/CreateDatabaseStatement.h"
#include "statements/CreateTableStatement.h"
#include "statements/DescribeStatement.h"
#include "statements/InsertStatement.h"
#include "statements/SelectStatement.h"
#include "statements/DeleteStatement.h"
#include "statements/Statement.h"
#include "statements/UseDatabaseStatement.h"
#include "storage/BufferPoolManager.h"
#include "storage/SlottedPage.h"
#include "index/BPlusTree.h"
#include "index/BPlusTreePage.h"
#include <cstring>
#include <iostream>
#include <stdexcept>
#include "concurrency/TransactionManager.h"
#include "concurrency/LockManager.h"
#include "storage/LogManager.h"
#include "storage/LogRecord.h"

Engine::Engine() : active(false), activeDbName("") {}
Engine::~Engine() = default;

void Engine::useDatabase(const std::string &name) {
  activeDbName = name;
  active = true;

  std::string fileName = name + ".db";
  diskManager = std::make_unique<DiskManager>(fileName);
  bpm = std::make_unique<BufferPoolManager>(10, diskManager.get());
  catalog = std::make_unique<Catalog>(bpm.get());
  lockManager = std::make_unique<LockManager>();
  txnManager = std::make_unique<TransactionManager>(lockManager.get());
  logManager = std::make_unique<LogManager>(activeDbName + ".log");
}

void Engine::createDatabase(const std::string &name) {
  std::string fileName = name + ".db";
  // Instantiating DiskManager will automatically create the file if it doesn't
  // exist
  DiskManager dm(fileName);
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

    std::unordered_map<std::string, page_id_t> indexes;
    for (auto* col : createTableStmt->columns) {
      if (col->type == ColumnType::INTEGER && col->primaryKey) {
        page_id_t rootId;
        Page* rootPage = bpm->newPage(&rootId);
        auto* leaf = reinterpret_cast<BPlusTreeLeafPage*>(rootPage->getData());
        leaf->init(rootId, -1);
        bpm->unpinPage(rootId, true);
        indexes[col->name] = rootId;
      }
    }

    Schema schema(std::move(createTableStmt->columns));
    schema.indexes = indexes;
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
    bool auto_commit = false;
    if (currentTxn == nullptr) {
      currentTxn = txnManager->Begin();
      logManager->AppendLogRecord(LogRecord(currentTxn->GetTransactionId(), LogRecordType::BEGIN));
      auto_commit = true;
    }
    auto insertStmt = static_cast<InsertStatement *>(statement);
    executeInsert(insertStmt);
    if (auto_commit) {
      logManager->AppendLogRecord(LogRecord(currentTxn->GetTransactionId(), LogRecordType::COMMIT));
      logManager->Flush();
      txnManager->Commit(currentTxn);
      delete currentTxn;
      currentTxn = nullptr;
    }
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
    bool auto_commit = false;
    if (currentTxn == nullptr) {
      currentTxn = txnManager->Begin();
      logManager->AppendLogRecord(LogRecord(currentTxn->GetTransactionId(), LogRecordType::BEGIN));
      auto_commit = true;
    }
    auto selectStmt = static_cast<SelectStatement *>(statement);
    executeSelect(selectStmt);
    if (auto_commit) {
      logManager->AppendLogRecord(LogRecord(currentTxn->GetTransactionId(), LogRecordType::COMMIT));
      logManager->Flush();
      txnManager->Commit(currentTxn);
      delete currentTxn;
      currentTxn = nullptr;
    }
    break;
  }
  case Statement::StatementType::DELETE_STATEMENT: {
    if (!active) {
      std::cerr << "Error: No database active. Use 'USE DATABASE' first.\n";
      break;
    }
    bool auto_commit = false;
    if (currentTxn == nullptr) {
      currentTxn = txnManager->Begin();
      logManager->AppendLogRecord(LogRecord(currentTxn->GetTransactionId(), LogRecordType::BEGIN));
      auto_commit = true;
    }
    auto deleteStmt = static_cast<DeleteStatement *>(statement);
    executeDelete(deleteStmt);
    if (auto_commit) {
      logManager->AppendLogRecord(LogRecord(currentTxn->GetTransactionId(), LogRecordType::COMMIT));
      logManager->Flush();
      txnManager->Commit(currentTxn);
      delete currentTxn;
      currentTxn = nullptr;
    }
    break;
  }
  case Statement::StatementType::BEGIN_TRANSACTION: {
    if (!active) {
      std::cerr << "Error: No database active. Use 'USE DATABASE' first.\n";
      break;
    }
    if (currentTxn != nullptr) {
      std::cerr << "Error: Transaction already in progress.\n";
      break;
    }
    currentTxn = txnManager->Begin();
    logManager->AppendLogRecord(LogRecord(currentTxn->GetTransactionId(), LogRecordType::BEGIN));
    std::cout << "Transaction started.\n";
    break;
  }
  case Statement::StatementType::COMMIT_TRANSACTION: {
    if (!active) {
      std::cerr << "Error: No database active. Use 'USE DATABASE' first.\n";
      break;
    }
    if (currentTxn == nullptr) {
      std::cerr << "Error: No active transaction.\n";
      break;
    }
    logManager->AppendLogRecord(LogRecord(currentTxn->GetTransactionId(), LogRecordType::COMMIT));
    logManager->Flush();
    txnManager->Commit(currentTxn);
    delete currentTxn;
    currentTxn = nullptr;
    std::cout << "Transaction committed.\n";
    break;
  }
  case Statement::StatementType::ROLLBACK_TRANSACTION: {
    if (!active) {
      std::cerr << "Error: No database active. Use 'USE DATABASE' first.\n";
      break;
    }
    if (currentTxn == nullptr) {
      std::cerr << "Error: No active transaction.\n";
      break;
    }
    logManager->AppendLogRecord(LogRecord(currentTxn->GetTransactionId(), LogRecordType::ABORT));
    logManager->Flush();
    txnManager->Abort(currentTxn);
    delete currentTxn;
    currentTxn = nullptr;
    std::cout << "Transaction rolled back.\n";
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
  if (!lockManager->LockTable(currentTxn, LockMode::EXCLUSIVE, stmt->tableName)) {
    std::cerr << "Error: Could not acquire lock for insert.\n";
    return;
  }

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
  std::unordered_map<std::string, int32_t> insertedInts;

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
      insertedInts[col->name] = intVal;
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

    auto updateIndexes = [&](int32_t sId, page_id_t pId) {
      bool schemaChanged = false;
      for (const auto& [colName, rootId] : schema->indexes) {
        if (insertedInts.count(colName)) {
           BPlusTree tree(rootId, bpm.get());
           tree.insert(insertedInts[colName], {pId, sId});
           if (tree.getRootPageId() != rootId) {
             schema->indexes[colName] = tree.getRootPageId();
             schemaChanged = true;
           }
        }
      }
      if (schemaChanged) {
        catalog->updateTable(stmt->tableName, *schema);
      }
    };

    if (slotId >= 0) {
      // Successfully inserted
      updateIndexes(slotId, currentPageId);
      logManager->AppendLogRecord(LogRecord(currentTxn->GetTransactionId(), LogRecordType::INSERT, stmt->tableName, currentPageId, slotId, tupleData));
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

      updateIndexes(slotId, newPageId);
      logManager->AppendLogRecord(LogRecord(currentTxn->GetTransactionId(), LogRecordType::INSERT, stmt->tableName, newPageId, slotId, tupleData));
      bpm->unpinPage(newPageId, true);
      std::cout << "Inserted 1 row into '" << stmt->tableName
                << "' (allocated new page " << newPageId << ").\n";
      return;
    }
  }
}

void Engine::executeSelect(SelectStatement *stmt) {
  if (!lockManager->LockTable(currentTxn, LockMode::SHARED, stmt->tableName)) {
    std::cerr << "Error: Could not acquire lock for select.\n";
    return;
  }

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

  std::vector<RecordId> indexedRecords;
  bool useIndex = false;

  if (stmt->filter != nullptr) {
    if (auto eqExpr = dynamic_cast<EqualityExpression*>(stmt->filter)) {
      if (auto idExpr = dynamic_cast<IdentifierExpression*>(eqExpr->left)) {
        if (auto litExpr = dynamic_cast<LiteralExpression*>(eqExpr->right)) {
          if (schema->indexes.count(idExpr->columnName) && litExpr->value.index() == 1) { // 1 is int
             useIndex = true;
             int targetVal = std::get<int>(litExpr->value);
             BPlusTree tree(schema->indexes[idExpr->columnName], bpm.get());
             auto res = tree.search(targetVal);
             if (res.has_value()) {
               indexedRecords.push_back(res.value());
             }
             std::cout << "[Using B+ Tree Index on '" << idExpr->columnName << "']\n";
          }
        }
      }
    }
  }

  auto processTuple = [&](std::string tupleData) -> bool {
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
          return false;
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
      return true;
  };

  if (useIndex) {
    for (const auto& rid : indexedRecords) {
      Page* page = bpm->fetchPage(rid.pageId);
      if (page) {
        SlottedPage sp(page);
        std::string tupleData = sp.getTuple(rid.slotId);
        if (!tupleData.empty()) {
           if (!processTuple(tupleData)) {
              bpm->unpinPage(rid.pageId, false);
              return;
           }
        }
        bpm->unpinPage(rid.pageId, false);
      }
    }
  } else {
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

        if (!processTuple(tupleData)) {
          bpm->unpinPage(page->getPageId(), false);
          return;
        }
      }

      currentPageId = sp.getNextPageId();
      bpm->unpinPage(page->getPageId(), false);
    }
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

void Engine::executeDelete(DeleteStatement *stmt) {
  if (!lockManager->LockTable(currentTxn, LockMode::EXCLUSIVE, stmt->tableName)) {
    std::cerr << "Error: Could not acquire lock for delete.\n";
    return;
  }

  Schema *schema = catalog->getTable(stmt->tableName);
  if (!schema) {
    std::cerr << "Error: Table '" << stmt->tableName << "' does not exist.\n";
    return;
  }

  int deletedCount = 0;
  std::vector<RecordId> indexedRecords;
  bool useIndex = false;

  if (stmt->filter != nullptr) {
    if (auto eqExpr = dynamic_cast<EqualityExpression*>(stmt->filter)) {
      if (auto idExpr = dynamic_cast<IdentifierExpression*>(eqExpr->left)) {
        if (auto litExpr = dynamic_cast<LiteralExpression*>(eqExpr->right)) {
          if (schema->indexes.count(idExpr->columnName) && litExpr->value.index() == 1) { // 1 is int
             useIndex = true;
             int targetVal = std::get<int>(litExpr->value);
             BPlusTree tree(schema->indexes[idExpr->columnName], bpm.get());
             auto res = tree.search(targetVal);
             if (res.has_value()) {
               indexedRecords.push_back(res.value());
             }
          }
        }
      }
    }
  }

  auto processTuple = [&](std::string tupleData, page_id_t pageId, int32_t slotId, SlottedPage& sp) -> bool {
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
          return false;
        }
      }

      if (matches) {
        logManager->AppendLogRecord(LogRecord(currentTxn->GetTransactionId(), LogRecordType::DELETE, stmt->tableName, pageId, slotId, tupleData));
        sp.deleteTuple(slotId);
        deletedCount++;
        
        bool schemaChanged = false;
        for (const auto& [colName, rootId] : schema->indexes) {
          if (row.count(colName) && row[colName].index() == 1) { // 1 is int
             BPlusTree tree(rootId, bpm.get());
             tree.remove(std::get<int>(row[colName]));
             if (tree.getRootPageId() != rootId) {
               schema->indexes[colName] = tree.getRootPageId();
               schemaChanged = true;
             }
          }
        }
        if (schemaChanged) {
          catalog->updateTable(stmt->tableName, *schema);
        }
      }
      return true;
  };

  if (useIndex) {
    for (const auto& rid : indexedRecords) {
      Page* page = bpm->fetchPage(rid.pageId);
      if (page) {
        SlottedPage sp(page);
        std::string tupleData = sp.getTuple(rid.slotId);
        if (!tupleData.empty()) {
           bool ok = processTuple(tupleData, rid.pageId, rid.slotId, sp);
           if (!ok) {
              bpm->unpinPage(rid.pageId, false);
              return;
           }
        }
        bpm->unpinPage(rid.pageId, true);
      }
    }
  } else {
    page_id_t currentPageId = schema->firstPageId;
    while (currentPageId != -1) {
      Page *page = bpm->fetchPage(currentPageId);
      if (!page) {
        std::cerr << "Error: Could not fetch data page " << currentPageId << ".\n";
        return;
      }

      SlottedPage sp(page);
      uint16_t slotCount = sp.getSlotCount();
      bool modified = false;

      for (slot_id_t i = 0; i < slotCount; ++i) {
        std::string tupleData = sp.getTuple(i);
        if (tupleData.empty())
          continue;

        int prevDelCount = deletedCount;
        if (!processTuple(tupleData, currentPageId, i, sp)) {
          bpm->unpinPage(page->getPageId(), modified);
          return;
        }
        if (deletedCount > prevDelCount) {
            modified = true;
        }
      }

      currentPageId = sp.getNextPageId();
      bpm->unpinPage(page->getPageId(), modified);
    }
  }

  std::cout << "Query OK, " << deletedCount << " rows affected.\n";
}
