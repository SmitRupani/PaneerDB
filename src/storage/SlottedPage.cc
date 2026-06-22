#include "SlottedPage.h"
#include <cstring>

SlottedPage::SlottedPage(Page* page) : page_(page), data_(page->getData()) {}

void SlottedPage::init() {
    setFreeSpacePointer(PAGE_SIZE);
    setSlotCount(0);
    setNextPageId(-1);
}

uint16_t SlottedPage::getFreeSpacePointer() const {
    uint16_t ptr;
    std::memcpy(&ptr, data_ + FREE_SPACE_OFFSET, sizeof(uint16_t));
    return ptr;
}

void SlottedPage::setFreeSpacePointer(uint16_t ptr) {
    std::memcpy(data_ + FREE_SPACE_OFFSET, &ptr, sizeof(uint16_t));
}

uint16_t SlottedPage::getSlotCount() const {
    uint16_t count;
    std::memcpy(&count, data_ + SLOT_COUNT_OFFSET, sizeof(uint16_t));
    return count;
}

void SlottedPage::setSlotCount(uint16_t count) {
    std::memcpy(data_ + SLOT_COUNT_OFFSET, &count, sizeof(uint16_t));
}

page_id_t SlottedPage::getNextPageId() const {
    page_id_t pageId;
    std::memcpy(&pageId, data_ + NEXT_PAGE_OFFSET, sizeof(page_id_t));
    return pageId;
}

void SlottedPage::setNextPageId(page_id_t pageId) {
    std::memcpy(data_ + NEXT_PAGE_OFFSET, &pageId, sizeof(page_id_t));
}

Slot SlottedPage::getSlot(slot_id_t slotId) const {
    Slot slot;
    std::memcpy(&slot, data_ + HEADER_SIZE + (slotId * SLOT_SIZE), sizeof(Slot));
    return slot;
}

void SlottedPage::setSlot(slot_id_t slotId, const Slot& slot) {
    std::memcpy(data_ + HEADER_SIZE + (slotId * SLOT_SIZE), &slot, sizeof(Slot));
}

uint16_t SlottedPage::getFreeSpace() const {
    uint16_t freeSpacePtr = getFreeSpacePointer();
    uint16_t slotCount = getSlotCount();
    uint16_t nextSlotOffset = HEADER_SIZE + (slotCount * SLOT_SIZE);

    if (freeSpacePtr >= nextSlotOffset) {
        return freeSpacePtr - nextSlotOffset;
    }
    return 0;
}

int32_t SlottedPage::insertTuple(const char* tupleData, uint16_t tupleLen) {
    uint16_t freeSpace = getFreeSpace();
    
    // We need space for the tuple itself + 4 bytes for a new slot
    if (freeSpace < tupleLen + SLOT_SIZE) {
        return -1; // Not enough space
    }

    // Determine where to write the tuple
    uint16_t freeSpacePtr = getFreeSpacePointer();
    uint16_t newTupleOffset = freeSpacePtr - tupleLen;
    
    // Write the tuple data backwards from the free space pointer
    std::memcpy(data_ + newTupleOffset, tupleData, tupleLen);
    
    // Update the free space pointer
    setFreeSpacePointer(newTupleOffset);

    // Create the slot
    uint16_t slotCount = getSlotCount();
    Slot newSlot = {newTupleOffset, tupleLen};
    setSlot(slotCount, newSlot);
    
    // Update slot count
    setSlotCount(slotCount + 1);

    // Return the assigned slot ID
    return slotCount;
}

std::string SlottedPage::getTuple(slot_id_t slotId) const {
    if (slotId >= getSlotCount()) {
        return "";
    }

    Slot slot = getSlot(slotId);

    // Lazy deleted tuple
    if (slot.offset == 0 && slot.length == 0) {
        return "";
    }

    return std::string(data_ + slot.offset, slot.length);
}

void SlottedPage::deleteTuple(slot_id_t slotId) {
    if (slotId >= getSlotCount()) {
        return;
    }
    // Lazy delete by setting offset and length to 0
    Slot emptySlot = {0, 0};
    setSlot(slotId, emptySlot);
}
