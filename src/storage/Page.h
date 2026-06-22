#ifndef PAGE_H
#define PAGE_H

#include "DiskManager/DiskManager.h" // For PAGE_SIZE and page_id_t
#include <cstring>

class Page {
public:
    Page() { resetMemory(); }
    ~Page() = default;

    inline char* getData() { return data; }
    inline page_id_t getPageId() const { return pageId; }
    inline int getPinCount() const { return pinCount; }
    inline bool isDirty() const { return dirty; }

protected:
    friend class BufferPoolManager;
    friend class SlottedPage;

    inline void resetMemory() { std::memset(data, 0, PAGE_SIZE); }

    char data[PAGE_SIZE]{};
    page_id_t pageId = -1; // -1 represents an invalid or unallocated page
    int pinCount = 0;
    bool dirty = false;
};

#endif // PAGE_H
