#include "index/BPlusTree.h"
#include <iostream>

BPlusTree::BPlusTree(page_id_t rootPageIdA, BufferPoolManager *bpmA) : rootPageId(rootPageIdA), bpm(bpmA) {}

page_id_t BPlusTree::findLeafPage(int key) {
  page_id_t currPageId = rootPageId;
  while (true) {
    Page *page = bpm->fetchPage(currPageId);
    if (!page) {
      return -1;
    }

    auto *header = reinterpret_cast<BPlusTreePageHeader *>(page->getData());
    if (header->isLeaf) {
      bpm->unpinPage(currPageId, false);
      return currPageId;
    }

    auto *internal = reinterpret_cast<BPlusTreeInternalPage *>(page->getData());
    page_id_t nextPage = internal->array[0].value;
    for (uint32_t i = 1; i < internal->header.size; ++i) {
      if (key < internal->array[i].key) {
        break;
      }
      nextPage = internal->array[i].value;
    }

    bpm->unpinPage(currPageId, false);
    currPageId = nextPage;
  }
}

std::optional<RecordId> BPlusTree::search(int key) {
  page_id_t leafPageId = findLeafPage(key);
  if (leafPageId == -1) return std::nullopt;

  Page *page = bpm->fetchPage(leafPageId);
  if (!page) return std::nullopt;

  auto *leaf = reinterpret_cast<BPlusTreeLeafPage *>(page->getData());

  // Binary search for exact match
  int left = 0;
  int right = leaf->header.size - 1;
  std::optional<RecordId> result = std::nullopt;

  while (left <= right) {
    int mid = left + (right - left) / 2;
    if (leaf->array[mid].key == key) {
      result = leaf->array[mid].value;
      break;
    } else if (leaf->array[mid].key < key) {
      left = mid + 1;
    } else {
      right = mid - 1;
    }
  }

  bpm->unpinPage(leafPageId, false);
  return result;
}

bool BPlusTree::remove(int key) {
  page_id_t leafPageId = findLeafPage(key);
  if (leafPageId == -1) return false;

  Page *page = bpm->fetchPage(leafPageId);
  if (!page) return false;

  auto *leaf = reinterpret_cast<BPlusTreeLeafPage *>(page->getData());

  bool found = false;
  uint32_t removeIdx = 0;
  for (uint32_t i = 0; i < leaf->header.size; ++i) {
    if (leaf->array[i].key == key) {
      found = true;
      removeIdx = i;
      break;
    }
  }

  if (found) {
    for (uint32_t i = removeIdx; i < leaf->header.size - 1; ++i) {
      leaf->array[i] = leaf->array[i + 1];
    }
    leaf->header.size--;
    bpm->unpinPage(leafPageId, true);
    return true;
  }

  bpm->unpinPage(leafPageId, false);
  return false;
}

bool BPlusTree::insert(int key, const RecordId &value) {
  if (search(key).has_value()) {
    return false;
  }

  page_id_t leafPageId = findLeafPage(key);
  if (leafPageId == -1) return false;

  Page *page = bpm->fetchPage(leafPageId);
  if (!page) return false;

  auto *leaf = reinterpret_cast<BPlusTreeLeafPage *>(page->getData());
  insertIntoLeaf(key, value, leaf);

  if (leaf->header.size > leaf->header.maxSize) {
    splitLeaf(leaf);
  }

  bpm->unpinPage(leafPageId, true);
  return true;
}

bool BPlusTree::insertIntoLeaf(int key, const RecordId &value, BPlusTreeLeafPage *leaf) {
  uint32_t insertIdx = 0;
  while (insertIdx < leaf->header.size && leaf->array[insertIdx].key < key) {
    insertIdx++;
  }

  for (uint32_t i = leaf->header.size; i > insertIdx; --i) {
    leaf->array[i] = leaf->array[i - 1];
  }

  leaf->array[insertIdx].key = key;
  leaf->array[insertIdx].value = value;
  leaf->header.size++;

  return true;
}

void BPlusTree::splitLeaf(BPlusTreeLeafPage *leaf) {
  page_id_t newPageId;
  Page *newPage = bpm->newPage(&newPageId);
  auto *newLeaf = reinterpret_cast<BPlusTreeLeafPage *>(newPage->getData());
  newLeaf->init(newPageId, leaf->header.parentPageId, leaf->header.maxSize);

  uint32_t splitIdx = leaf->header.size / 2;
  uint32_t moveCount = leaf->header.size - splitIdx;

  for (uint32_t i = 0; i < moveCount; ++i) {
    newLeaf->array[i] = leaf->array[splitIdx + i];
  }

  newLeaf->header.size = moveCount;
  leaf->header.size = splitIdx;

  newLeaf->nextPageId = leaf->nextPageId;
  leaf->nextPageId = newPageId;
  newLeaf->prevPageId = leaf->header.pageId;

  if (newLeaf->nextPageId != -1) {
    Page *nextPage = bpm->fetchPage(newLeaf->nextPageId);
    if (nextPage) {
      auto *nextLeaf = reinterpret_cast<BPlusTreeLeafPage *>(nextPage->getData());
      nextLeaf->prevPageId = newPageId;
      bpm->unpinPage(newLeaf->nextPageId, true);
    }
  }

  int middleKey = newLeaf->array[0].key;
  insertIntoParent(reinterpret_cast<BPlusTreePageHeader *>(leaf), middleKey, reinterpret_cast<BPlusTreePageHeader *>(newLeaf));

  bpm->unpinPage(newPageId, true);
}

void BPlusTree::insertIntoParent(BPlusTreePageHeader *oldNode, int key, BPlusTreePageHeader *newNode) {
  if (oldNode->parentPageId == -1) {
    page_id_t newRootId;
    Page *newRootPage = bpm->newPage(&newRootId);
    auto *newRoot = reinterpret_cast<BPlusTreeInternalPage *>(newRootPage->getData());
    newRoot->init(newRootId, -1, B_PLUS_TREE_INTERNAL_PAGE_MAX_SIZE);

    newRoot->array[0].value = oldNode->pageId;
    newRoot->array[1].key = key;
    newRoot->array[1].value = newNode->pageId;
    newRoot->header.size = 2;

    oldNode->parentPageId = newRootId;
    newNode->parentPageId = newRootId;

    rootPageId = newRootId;

    bpm->unpinPage(newRootId, true);
    return;
  }

  page_id_t parentPageId = oldNode->parentPageId;
  Page *parentPage = bpm->fetchPage(parentPageId);
  auto *parent = reinterpret_cast<BPlusTreeInternalPage *>(parentPage->getData());

  uint32_t insertIdx = 1;
  while (insertIdx < parent->header.size && parent->array[insertIdx].key < key) {
    insertIdx++;
  }

  for (uint32_t i = parent->header.size; i > insertIdx; --i) {
    parent->array[i] = parent->array[i - 1];
  }

  parent->array[insertIdx].key = key;
  parent->array[insertIdx].value = newNode->pageId;
  parent->header.size++;

  if (parent->header.size > parent->header.maxSize) {
    splitInternal(parent);
  }

  bpm->unpinPage(parentPageId, true);
}

void BPlusTree::splitInternal(BPlusTreeInternalPage *internal) {
  page_id_t newPageId;
  Page *newPage = bpm->newPage(&newPageId);
  auto *newInternal = reinterpret_cast<BPlusTreeInternalPage *>(newPage->getData());
  newInternal->init(newPageId, internal->header.parentPageId, internal->header.maxSize);

  uint32_t splitIdx = internal->header.size / 2;
  int middleKey = internal->array[splitIdx].key;

  uint32_t moveCount = internal->header.size - splitIdx - 1;

  newInternal->array[0].value = internal->array[splitIdx].value;

  Page *childPage = bpm->fetchPage(newInternal->array[0].value);
  if (childPage) {
    auto *childHeader = reinterpret_cast<BPlusTreePageHeader *>(childPage->getData());
    childHeader->parentPageId = newPageId;
    bpm->unpinPage(childPage->getPageId(), true);
  }

  for (uint32_t i = 0; i < moveCount; ++i) {
    newInternal->array[i + 1] = internal->array[splitIdx + 1 + i];

    Page *cp = bpm->fetchPage(newInternal->array[i + 1].value);
    if (cp) {
      auto *ch = reinterpret_cast<BPlusTreePageHeader *>(cp->getData());
      ch->parentPageId = newPageId;
      bpm->unpinPage(cp->getPageId(), true);
    }
  }

  newInternal->header.size = moveCount + 1;
  internal->header.size = splitIdx;

  insertIntoParent(reinterpret_cast<BPlusTreePageHeader *>(internal), middleKey, reinterpret_cast<BPlusTreePageHeader *>(newInternal));

  bpm->unpinPage(newPageId, true);
}
