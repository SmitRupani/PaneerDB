#include "Engine.h"
#include "statements/Statement.h"
#include "statements/UseDatabaseStatement.h"
#include "statements/CreateDatabaseStatement.h"
#include "statements/CreateTableStatement.h"
#include "statements/InsertStatement.h"
#include "statements/DescribeStatement.h"
#include "DiskManager/DiskManager.h"
#include "storage/BufferPoolManager.h"
#include "storage/SlottedPage.h"
#include "catalog/Catalog.h"
#include <stdexcept>
#include <iostream>
#include <cstring>

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
    case Statement::StatementType::DOT_TABLES: {
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
      auto insertStmt = static_cast<InsertStatement*>(statement);
      executeInsert(insertStmt);
      break;
    }
    case Statement::StatementType::DESCRIBE_TABLE: {
      if (!active) {
        std::cerr << "Error: No database active. Use 'USE DATABASE' first.\n";
        break;
      }
      auto descStmt = static_cast<DescribeStatement*>(statement);
      Schema* schema = catalog->getTable(descStmt->tableName);
      if (schema) {
        schema->print();
      } else {
        std::cerr << "Error: Table '" << descStmt->tableName << "' does not exist.\n";
      }
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

void Engine::executeInsert(InsertStatement* stmt) {
  Schema* schema = catalog->getTable(stmt->tableName);
  if (!schema) {
    std::cerr << "Error: Table '" << stmt->tableName << "' does not exist.\n";
    return;
  }

  // Determine column mapping
  // If stmt->columns is empty, we expect values for all columns in order
  std::vector<Column*> insertColumns;
  if (stmt->columns.empty()) {
    insertColumns = schema->columns;
  } else {
    for (const auto& colName : stmt->columns) {
      bool found = false;
      for (auto* schemaCol : schema->columns) {
        if (schemaCol->name == colName) {
          insertColumns.push_back(schemaCol);
          found = true;
          break;
        }
      }
      if (!found) {
        std::cerr << "Error: Column '" << colName << "' does not exist in table '" << stmt->tableName << "'.\n";
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
    Column* col = insertColumns[i];
    Value val = stmt->values[i]->solve();

    if (col->type == ColumnType::INTEGER) {
      if (val.index() != 1) { // 1 is int
        std::cerr << "Error: Expected integer value for column '" << col->name << "'.\n";
        return;
      }
      int32_t intVal = std::get<int>(val);
      tupleData.append(reinterpret_cast<const char*>(&intVal), sizeof(intVal));
    } else if (col->type == ColumnType::VARCHAR) {
      if (val.index() != 0) { // 0 is std::string
        std::cerr << "Error: Expected string value for column '" << col->name << "'.\n";
        return;
      }
      std::string strVal = std::get<std::string>(val);
      
      // Need to cast col to VarcharColumn to get varcharLength
      auto* varcharCol = static_cast<VarcharColumn*>(col);
      if (strVal.length() > varcharCol->varcharLength) {
        std::cerr << "Error: Value too long for column '" << col->name << "' (max " << varcharCol->varcharLength << ").\n";
        return;
      }
      
      uint32_t strLen = strVal.length();
      tupleData.append(reinterpret_cast<const char*>(&strLen), sizeof(strLen));
      tupleData.append(strVal);
    }
  }

  // Insert into data page
  page_id_t currentPageId = schema->firstPageId;
  while (true) {
    Page* page = bpm->fetchPage(currentPageId);
    if (!page) {
      std::cerr << "Error: Could not fetch data page " << currentPageId << ".\n";
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
      Page* newPage = bpm->newPage(&newPageId);
      if (!newPage) {
        std::cerr << "Error: Could not allocate new page for table data.\n";
        bpm->unpinPage(currentPageId, false);
        return;
      }
      
      SlottedPage newSp(newPage);
      newSp.init();
      
      // Link current page to new page
      sp.setNextPageId(newPageId);
      bpm->unpinPage(currentPageId, true); // Dirty because we updated nextPageId
      
      // Try inserting into the new page
      slotId = newSp.insertTuple(tupleData.data(), tupleData.size());
      if (slotId < 0) {
        std::cerr << "Error: Tuple is too large to fit in an empty page.\n";
        bpm->unpinPage(newPageId, false);
        return;
      }
      
      bpm->unpinPage(newPageId, true);
      std::cout << "Inserted 1 row into '" << stmt->tableName << "' (allocated new page " << newPageId << ").\n";
      return;
    }
  }
}
