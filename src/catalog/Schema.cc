#include "catalog/Schema.h"
#include <cstring>
#include <utility>
#include <cstdint>

Schema::Schema(std::vector<Column*> columnsA, page_id_t firstPageIdA) : columns(std::move(columnsA)), firstPageId(firstPageIdA) {}

Schema::~Schema() {
  for (auto* col : columns) {
    delete col;
  }
}

Schema::Schema(Schema&& other) noexcept : columns(std::move(other.columns)), firstPageId(other.firstPageId) {
  other.columns.clear();
}

Schema& Schema::operator=(Schema&& other) noexcept {
  if (this != &other) {
    for (auto* col : columns) {
      delete col;
    }
    columns = std::move(other.columns);
    firstPageId = other.firstPageId;
    indexes = std::move(other.indexes);
    other.columns.clear();
    other.indexes.clear();
  }
  return *this;
}

std::string Schema::serialize() const {
  std::string buf;
  
  // Serialize firstPageId
  buf.append(reinterpret_cast<const char*>(&firstPageId), sizeof(firstPageId));
  
  uint16_t numColumns = columns.size();
  buf.append(reinterpret_cast<const char*>(&numColumns), sizeof(numColumns));

  for (const auto* col : columns) {
    std::string colBuf = col->serialize();
    buf.append(colBuf);
  }

  uint16_t numIndexes = indexes.size();
  buf.append(reinterpret_cast<const char*>(&numIndexes), sizeof(numIndexes));
  for (const auto& [colName, rootId] : indexes) {
    uint32_t nameLen = colName.length();
    buf.append(reinterpret_cast<const char*>(&nameLen), sizeof(nameLen));
    buf.append(colName);
    buf.append(reinterpret_cast<const char*>(&rootId), sizeof(rootId));
  }

  return buf;
}

Schema* Schema::deserialize(const std::string& data) {
  size_t offset = 0;
  
  page_id_t firstPageId;
  std::memcpy(&firstPageId, data.data() + offset, sizeof(firstPageId));
  offset += sizeof(firstPageId);
  
  uint16_t numColumns;
  std::memcpy(&numColumns, data.data() + offset, sizeof(numColumns));
  offset += sizeof(numColumns);

  std::vector<Column*> columns;
  for (uint16_t i = 0; i < numColumns; ++i) {
    Column* col = Column::deserialize(data, offset);
    columns.push_back(col);
  }

  Schema* schema = new Schema(std::move(columns), firstPageId);

  uint16_t numIndexes = 0;
  if (offset < data.size()) {
    std::memcpy(&numIndexes, data.data() + offset, sizeof(numIndexes));
    offset += sizeof(numIndexes);

    for (uint16_t i = 0; i < numIndexes; ++i) {
      uint32_t nameLen;
      std::memcpy(&nameLen, data.data() + offset, sizeof(nameLen));
      offset += sizeof(nameLen);

      std::string colName(data.data() + offset, nameLen);
      offset += nameLen;

      page_id_t rootId;
      std::memcpy(&rootId, data.data() + offset, sizeof(rootId));
      offset += sizeof(rootId);

      schema->indexes[colName] = rootId;
    }
  }

  return schema;
}

void Schema::print() const {
  for (const auto* col : columns) {
    col->print();
  }
}
