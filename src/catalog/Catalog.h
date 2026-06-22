#ifndef CATALOG_H
#define CATALOG_H

#include "catalog/Schema.h"
#include "storage/BufferPoolManager.h"
#include <string>
#include <unordered_map>

class Catalog {
public:
  Catalog(BufferPoolManager *bpm);
  ~Catalog();

  bool createTable(const std::string &tableName, Schema &schema);
  Schema *getTable(const std::string &tableName);
  void showTables() const;

  static constexpr page_id_t CATALOG_PAGE_ID = 1;

private:
  BufferPoolManager *bpm;
  std::unordered_map<std::string, Schema *> tables;
};

#endif // CATALOG_H
