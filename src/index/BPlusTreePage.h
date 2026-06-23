#ifndef B_PLUS_TREE_PAGE_H
#define B_PLUS_TREE_PAGE_H

#include "DiskManager/DiskManager.h"
#include <cstdint>

struct RecordId {
  page_id_t pageId;
  int32_t slotId;

  bool operator==(const RecordId& other) const {
    return pageId == other.pageId && slotId == other.slotId;
  }
};

struct BPlusTreePageHeader {
  page_id_t pageId;
  page_id_t parentPageId;
  uint32_t size;
  uint32_t maxSize;
  bool isLeaf;
};

// Size calculation for max keys
constexpr int B_PLUS_TREE_LEAF_PAGE_MAX_SIZE = (PAGE_SIZE - sizeof(BPlusTreePageHeader) - 2 * sizeof(page_id_t)) / (sizeof(int) + sizeof(RecordId));
constexpr int B_PLUS_TREE_INTERNAL_PAGE_MAX_SIZE = (PAGE_SIZE - sizeof(BPlusTreePageHeader)) / (sizeof(int) + sizeof(page_id_t));

class BPlusTreeLeafPage {
public:
  void init(page_id_t pageId, page_id_t parentId = -1, int maxSize = B_PLUS_TREE_LEAF_PAGE_MAX_SIZE) {
    header.pageId = pageId;
    header.parentPageId = parentId;
    header.size = 0;
    header.maxSize = maxSize;
    header.isLeaf = true;
    nextPageId = -1;
    prevPageId = -1;
  }

  BPlusTreePageHeader header;
  page_id_t nextPageId;
  page_id_t prevPageId;

  struct Pair {
    int key;
    RecordId value;
  };

  Pair array[B_PLUS_TREE_LEAF_PAGE_MAX_SIZE];
};
static_assert(sizeof(BPlusTreeLeafPage) <= PAGE_SIZE, "BPlusTreeLeafPage is too large");

class BPlusTreeInternalPage {
public:
  void init(page_id_t pageId, page_id_t parentId = -1, int maxSize = B_PLUS_TREE_INTERNAL_PAGE_MAX_SIZE) {
    header.pageId = pageId;
    header.parentPageId = parentId;
    header.size = 0;
    header.maxSize = maxSize;
    header.isLeaf = false;
  }

  BPlusTreePageHeader header;

  struct Pair {
    int key;
    page_id_t value; // For index 0, key is invalid
  };

  Pair array[B_PLUS_TREE_INTERNAL_PAGE_MAX_SIZE];
};
static_assert(sizeof(BPlusTreeInternalPage) <= PAGE_SIZE, "BPlusTreeInternalPage is too large");

#endif // B_PLUS_TREE_PAGE_H
