#ifndef B_PLUS_TREE_H
#define B_PLUS_TREE_H

#include "index/BPlusTreePage.h"
#include "storage/BufferPoolManager.h"
#include <optional>
#include <vector>

class BPlusTree {
public:
  BPlusTree(page_id_t rootPageIdA, BufferPoolManager *bpmA);

  bool insert(int key, const RecordId &value);
  std::optional<RecordId> search(int key);
  bool remove(int key);

  page_id_t getRootPageId() const { return rootPageId; }

private:
  page_id_t rootPageId;
  BufferPoolManager *bpm;

  // Helper methods
  page_id_t findLeafPage(int key);
  
  bool insertIntoLeaf(int key, const RecordId &value, BPlusTreeLeafPage *leaf);
  void insertIntoParent(BPlusTreePageHeader *oldNode, int key, BPlusTreePageHeader *newNode);
  
  void splitLeaf(BPlusTreeLeafPage *leaf);
  void splitInternal(BPlusTreeInternalPage *internal);
};

#endif // B_PLUS_TREE_H
