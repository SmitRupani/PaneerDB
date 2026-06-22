#ifndef PAGE_H
#define PAGE_H

#include "DiskManager/DiskManager.h" // For PAGE_SIZE and page_id_t
#include <cstring>

class Page {
public:
    Page() { resetMemory(); }
    ~Page() = default;

    inline char* getData() { return data_; }
    inline page_id_t getPageId() const { return page_id_; }
    inline int getPinCount() const { return pin_count_; }
    inline bool isDirty() const { return is_dirty_; }

protected:
    friend class BufferPoolManager;
    friend class SlottedPage;

    inline void resetMemory() { std::memset(data_, 0, PAGE_SIZE); }

    char data_[PAGE_SIZE]{};
    page_id_t page_id_ = -1; // -1 represents an invalid or unallocated page
    int pin_count_ = 0;
    bool is_dirty_ = false;
};

#endif // PAGE_H
