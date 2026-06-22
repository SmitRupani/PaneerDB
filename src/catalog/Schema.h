#ifndef SCHEMA_H
#define SCHEMA_H

#include "DiskManager/DiskManager.h"
#include "statements/Column.h"
#include <string>
#include <vector>

class Schema {
public:
  Schema(std::vector<Column *> columnsA, page_id_t firstPageIdA = -1);
  ~Schema();

  // Disable copy constructor and assignment to avoid double-free of Column*
  Schema(const Schema &) = delete;
  Schema &operator=(const Schema &) = delete;

  // Allow move
  Schema(Schema &&) noexcept;
  Schema &operator=(Schema &&) noexcept;

  std::string serialize() const;
  static Schema *deserialize(const std::string &data);

  void print() const;

  std::vector<Column *> columns;
  page_id_t firstPageId;
};

#endif // SCHEMA_H
