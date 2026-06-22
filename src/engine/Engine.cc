#include "Engine.h"
#include "statements/Statement.h"
#include "statements/UseDatabaseStatement.h"
#include "statements/CreateDatabaseStatement.h"
#include "statements/CreateTableStatement.h"
#include "DiskManager/DiskManager.h"
#include "storage/BufferPoolManager.h"
#include "catalog/Catalog.h"
#include <stdexcept>
#include <iostream>

Engine::Engine() : active(false), activeDbName("") {}
Engine::~Engine() = default;

void Engine::useDatabase(const std::string &name) {
  activeDbName = name;
  active = true;

  std::string fileName = name + ".db";
  diskManager = std::make_unique<DiskManager>(fileName);
  bpm = std::make_unique<BufferPoolManager>(10, diskManager.get());
  catalog = std::make_unique<Catalog>(bpm.get());
  
  std::cout << "Using database: " << name << std::endl;
}

void Engine::createDatabase(const std::string &name) {
  std::string fileName = name + ".db";
  // Instantiating DiskManager will automatically create the file if it doesn't exist
  DiskManager dm(fileName);
  std::cout << "Created database: " << name << std::endl;
}

void Engine::execute(Statement* statement) {
  if (!statement) return;

  switch (statement->getType()) {
    case Statement::StatementType::USE_DATABASE: {
      auto useDbStmt = static_cast<UseDatabaseStatement*>(statement);
      useDatabase(useDbStmt->dbName);
      break;
    }
    case Statement::StatementType::CREATE_DATABASE: {
      auto createDbStmt = static_cast<CreateDatabaseStatement*>(statement);
      createDatabase(createDbStmt->dbName);
      break;
    }
    case Statement::StatementType::CREATE_TABLE: {
      if (!active) {
        std::cerr << "Error: No database active. Use 'USE DATABASE' first.\n";
        break;
      }
      auto createTableStmt = static_cast<CreateTableStatement*>(statement);
      Schema schema(std::move(createTableStmt->columns));
      createTableStmt->columns.clear(); // schema takes ownership
      
      catalog->createTable(createTableStmt->tableName, schema);
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
