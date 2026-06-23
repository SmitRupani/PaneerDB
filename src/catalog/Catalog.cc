#include "catalog/Catalog.h"
#include "storage/SlottedPage.h"
#include <cstring>
#include <iostream>

Catalog::Catalog(BufferPoolManager *bpm) : bpm(bpm) {
  DiskManager *diskManager = bpm->getDiskManager();

  if (diskManager->getNextPageId() == 0) {
    page_id_t metaPageId, catalogPageId;
    Page *metaPage = bpm->newPage(&metaPageId);       // Should be 0
    Page *catalogPage = bpm->newPage(&catalogPageId); // Should be 1

    if (metaPageId != 0 || catalogPageId != 1) {
      std::cerr << "Warning: Allocated pages didn't match expected Meta(0) and "
                   "Catalog(1)\n";
    }

    SlottedPage sp(catalogPage);
    sp.init();

    bpm->unpinPage(metaPageId, true);
    bpm->unpinPage(catalogPageId, true);
  } else {
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
        continue;

      size_t offset = 0;
      uint16_t nameLen;
      std::memcpy(&nameLen, tupleData.data() + offset, sizeof(nameLen));
      offset += sizeof(nameLen);

      std::string tableName = tupleData.substr(offset, nameLen);
      offset += nameLen;

      std::string schemaData = tupleData.substr(offset);
      Schema *schema = Schema::deserialize(schemaData);

      tables[tableName] = schema;
    }

    bpm->unpinPage(CATALOG_PAGE_ID, false);
  }
}

Catalog::~Catalog() {
  for (auto &pair : tables) {
    delete pair.second;
  }
}

bool Catalog::createTable(const std::string &tableName, Schema &schema) {
  if (tables.find(tableName) != tables.end()) {
    std::cerr << "Table '" << tableName << "' already exists.\n";
    return false;
  }

  page_id_t firstPageId;
  Page *firstPage = bpm->newPage(&firstPageId);
  if (!firstPage) {
    std::cerr << "Error: Could not allocate first page for table\n";
    return false;
  }
  SlottedPage spFirst(firstPage);
  spFirst.init();
  bpm->unpinPage(firstPageId, true);

  schema.firstPageId = firstPageId;

  Page *catalogPage = bpm->fetchPage(CATALOG_PAGE_ID);
  if (!catalogPage) {
    std::cerr << "Error: Could not fetch Catalog Page 1\n";
    return false;
  }
  SlottedPage sp(catalogPage);

  std::string tupleData;
  uint16_t nameLen = tableName.size();
  tupleData.append(reinterpret_cast<const char *>(&nameLen), sizeof(nameLen));
  tupleData.append(tableName);
  tupleData.append(schema.serialize());

  int32_t slotId = sp.insertTuple(tupleData.data(), tupleData.size());
  if (slotId < 0) {
    std::cerr
        << "Failed to insert table schema into catalog page. Page full?\n";
    bpm->unpinPage(CATALOG_PAGE_ID, false);
    return false;
  }

  bpm->unpinPage(CATALOG_PAGE_ID, true);

  std::string schemaStr = schema.serialize();
  tables[tableName] = Schema::deserialize(schemaStr);

  return true;
}

bool Catalog::updateTable(const std::string &tableName, Schema &schema) {
  if (tables.find(tableName) == tables.end()) {
    std::cerr << "Table '" << tableName << "' does not exist.\n";
    return false;
  }

  Page *catalogPage = bpm->fetchPage(CATALOG_PAGE_ID);
  if (!catalogPage) {
    return false;
  }
  SlottedPage sp(catalogPage);

  // Find old tuple and delete it
  uint16_t slotCount = sp.getSlotCount();
  for (uint16_t i = 0; i < slotCount; ++i) {
    std::string tupleData = sp.getTuple(i);
    if (tupleData.empty()) continue;

    size_t offset = 0;
    uint16_t nameLen;
    std::memcpy(&nameLen, tupleData.data() + offset, sizeof(nameLen));
    offset += sizeof(nameLen);

    std::string existingTableName = tupleData.substr(offset, nameLen);
    if (existingTableName == tableName) {
      sp.deleteTuple(i);
      break;
    }
  }

  std::string tupleData;
  uint16_t nameLen = tableName.size();
  tupleData.append(reinterpret_cast<const char *>(&nameLen), sizeof(nameLen));
  tupleData.append(tableName);
  tupleData.append(schema.serialize());

  int32_t slotId = sp.insertTuple(tupleData.data(), tupleData.size());
  if (slotId < 0) {
    std::cerr << "Failed to update table schema into catalog page. Page full?\n";
    bpm->unpinPage(CATALOG_PAGE_ID, false);
    return false;
  }

  bpm->unpinPage(CATALOG_PAGE_ID, true);

  std::string schemaStr = schema.serialize();
  delete tables[tableName];
  tables[tableName] = Schema::deserialize(schemaStr);

  return true;
}

Schema *Catalog::getTable(const std::string &tableName) {
  if (tables.find(tableName) != tables.end()) {
    return tables[tableName];
  }
  return nullptr;
}

void Catalog::showTables() const {
  if (tables.empty()) {
    std::cout << "No tables found in the database.\n";
    return;
  }
  for (const auto &pair : tables) {
    std::cout << pair.first << "\n";
  }
}
