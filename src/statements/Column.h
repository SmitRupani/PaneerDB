#ifndef COLUMN_H
#define COLUMN_H

#include <string>

enum class ColumnType { INTEGER, VARCHAR };

class Column {
public:
  Column(std::string nameA, ColumnType typeA, bool primaryKeyA, bool notNullA,
         bool uniqueA);
  virtual ~Column() = default;

  std::string name;
  ColumnType type;
  bool primaryKey;
  bool notNull;
  bool unique;

  virtual void print() const = 0;

  /*
   * Column Metadata:
   * - Name Length (uint16_t): Length of the column name.
   * - Name (variable): Column name.
   * - Type (uint8_t): 0 = INTEGER, 1 = VARCHAR.
   * - Primary Key (uint8_t): 0 = No, 1 = Yes.
   * - Not Null (uint8_t): 0 = No, 1 = Yes.
   * - Unique (uint8_t): 0 = No, 1 = Yes.
   * - Varchar Length (uint32_t): Present only if Type == VARCHAR.
   */
  virtual std::string serialize() const = 0;

  static Column *deserialize(const std::string &data, size_t &offset);
};

class IntegerColumn : public Column {
public:
  IntegerColumn(std::string nameA, bool primaryKeyA, bool notNullA,
                bool uniqueA);
  void print() const override;
  std::string serialize() const override;
};

class VarcharColumn : public Column {
public:
  VarcharColumn(std::string nameA, int lengthA, bool primaryKeyA, bool notNullA,
                bool uniqueA);
  void print() const override;
  std::string serialize() const override;

  int varcharLength;
};

#endif
