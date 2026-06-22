#include "BufferPoolManager.h"
#include <stdexcept>
#include <iostream>

BufferPoolManager::BufferPoolManager(size_t poolSize, DiskManager* diskManager)
    : poolSize(poolSize), diskManager(diskManager) {
    
    pages = new Page[this->poolSize];
    
    // Initially, all frames are free
    for (size_t i = 0; i < this->poolSize; ++i) {
        freeList.push_back(i);
    }

    // Register callback for when the background thread evicts a page
    replacer.setEvictCallback([this](int pageId, frame_id_t frameId) {
        this->onEvict(pageId, frameId);
    });
}

BufferPoolManager::~BufferPoolManager() {
    flushAllPages();
    delete[] pages;
}

void BufferPoolManager::onEvict(int pageId, frame_id_t frameId) {
    std::lock_guard<std::mutex> lockGuard(latch);
    
    // Check if the page is still in the table and maps to this frame
    if (pageTable.find(pageId) != pageTable.end() && pageTable[pageId] == frameId) {
        Page* page = &pages[frameId];
        
        // If it's somehow pinned, we shouldn't have evicted it. 
        // But the replacer only tracks unpinned pages.
        if (page->pinCount > 0) {
            std::cerr << "Warning: Background thread evicted a pinned page!\n";
            return; 
        }

        if (page->dirty) {
            diskManager->writePage(page->pageId, page->data);
            page->dirty = false;
        }

        pageTable.erase(pageId);
        page->pageId = -1;
        page->pinCount = 0;
        page->resetMemory();
        
        freeList.push_back(frameId);
    }
}

bool BufferPoolManager::getVictimFrame(frame_id_t* frameId) {
    // 1. Try free list
    if (!freeList.empty()) {
        *frameId = freeList.front();
        freeList.pop_front();
        return true;
    }
    
    // 2. Try to manually evict one from the cache synchronously
    auto victim = replacer.evictOne();
    if (victim.has_value()) {
        page_id_t evictPageId = victim->first;
        *frameId = victim->second;
        
        Page* page = &pages[*frameId];
        if (page->dirty) {
            diskManager->writePage(page->pageId, page->data);
            page->dirty = false;
        }
        
        pageTable.erase(evictPageId);
        page->pageId = -1;
        page->pinCount = 0;
        page->resetMemory();
        return true;
    }
    
    return false; // All frames pinned
}

Page* BufferPoolManager::fetchPage(page_id_t pageId) {
    std::lock_guard<std::mutex> lockGuard(latch);

    // 1. If page is already in the buffer pool
    if (pageTable.find(pageId) != pageTable.end()) {
        frame_id_t frameId = pageTable[pageId];
        Page* page = &pages[frameId];
        
        page->pinCount++;
        if (page->pinCount == 1) {
            replacer.remove(pageId); // It's pinned, remove from cache
        }
        return page;
    }

    // 2. Page is not in pool. Find a frame.
    frame_id_t frameId;
    if (!getVictimFrame(&frameId)) {
        return nullptr; // No frames available
    }

    // 3. Read from disk into frame
    Page* page = &pages[frameId];
    page->pageId = pageId;
    page->pinCount = 1;
    page->dirty = false;
    
    // DiskManager might throw if reading past EOF, but typical fetch is for existing pages
    diskManager->readPage(pageId, page->data);
    
    pageTable[pageId] = frameId;
    
    return page;
}

bool BufferPoolManager::unpinPage(page_id_t pageId, bool isDirty) {
    std::lock_guard<std::mutex> lockGuard(latch);
    
    if (pageTable.find(pageId) == pageTable.end()) {
        return false;
    }
    
    frame_id_t frameId = pageTable[pageId];
    Page* page = &pages[frameId];
    
    if (page->pinCount <= 0) {
        return false; // Already unpinned
    }
    
    page->pinCount--;
    if (isDirty) {
        page->dirty = true;
    }
    
    // If it becomes completely unpinned, add it to the eviction cache
    if (page->pinCount == 0) {
        replacer.put(pageId, frameId);
    }
    
    return true;
}

bool BufferPoolManager::flushPage(page_id_t pageId) {
    std::lock_guard<std::mutex> lockGuard(latch);
    
    if (pageTable.find(pageId) == pageTable.end()) {
        return false;
    }
    
    frame_id_t frameId = pageTable[pageId];
    Page* page = &pages[frameId];
    
    if (page->dirty) {
        diskManager->writePage(page->pageId, page->data);
        page->dirty = false;
    }
    
    return true;
}

Page* BufferPoolManager::newPage(page_id_t* pageId) {
    std::lock_guard<std::mutex> lockGuard(latch);
    
    frame_id_t frameId;
    if (!getVictimFrame(&frameId)) {
        return nullptr;
    }
    
    *pageId = diskManager->allocatePage();
    
    Page* page = &pages[frameId];
    page->pageId = *pageId;
    page->pinCount = 1;
    page->dirty = false; // or true if we want to ensure it's written eventually
    page->resetMemory();
    
    pageTable[*pageId] = frameId;
    
    return page;
}

bool BufferPoolManager::deletePage(page_id_t pageId) {
    std::lock_guard<std::mutex> lockGuard(latch);
    
    if (pageTable.find(pageId) == pageTable.end()) {
        return true;
    }
    
    frame_id_t frameId = pageTable[pageId];
    Page* page = &pages[frameId];
    
    if (page->pinCount > 0) {
        return false; // Cannot delete a pinned page
    }
    
    replacer.remove(pageId);
    pageTable.erase(pageId);
    page->pageId = -1;
    page->dirty = false;
    page->resetMemory();
    freeList.push_back(frameId);
    
    return true;
}

void BufferPoolManager::flushAllPages() {
    std::lock_guard<std::mutex> lockGuard(latch);
    
    for (auto const& [pageId, frameId] : pageTable) {
        Page* page = &pages[frameId];
        if (page->dirty) {
            diskManager->writePage(page->pageId, page->data);
            page->dirty = false;
        }
    }
}
