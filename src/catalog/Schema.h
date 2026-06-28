#ifndef SCHEMA_H
#define SCHEMA_H

#include "DiskManager/DiskManager.h"
#include "statements/Column.h"
#include <string>
#include <unordered_map>
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

  /*
   * Schema Metadata:
   * - First Page ID (page_id_t)
   * - Num Columns (uint16_t)
   * - [Columns Data]
   * - Num Indexes (uint16_t)
   * - [Indexes]:
   *     - Name Length (uint32_t)
   *     - Name (string)
   *     - Root Page ID (page_id_t)
   */
  std::string serialize() const;
  static Schema *deserialize(const std::string &data);

  void print() const;

  std::vector<Column *> columns;

  // Page id from which the table data starts
  page_id_t firstPageId;

  std::unordered_map<std::string, page_id_t> indexes;
};

#endif // SCHEMA_H
