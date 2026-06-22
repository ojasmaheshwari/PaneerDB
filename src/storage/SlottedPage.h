#ifndef SLOTTED_PAGE_H
#define SLOTTED_PAGE_H

#include "Page.h"
#include <cstdint>
#include <string>

// A slotted page is a wrapper around a raw 4KB Page to store tuples of variable size.
// Layout:
// [Header] -> [Slots] -> ... free space ... <- [Tuples]
// 
// Header Layout (4 bytes):
// - uint16_t freeSpacePointer (offset to where the next tuple can be written backwards)
// - uint16_t slotCount (number of slots currently in the slot array)
//
// Slot Layout (4 bytes per slot):
// - uint16_t tupleOffset
// - uint16_t tupleLength

struct Slot {
    uint16_t offset;
    uint16_t length;
};

using slot_id_t = uint16_t;

class SlottedPage {
public:
    explicit SlottedPage(Page* page);

    // Initializes a fresh page with empty header
    void init();

    // Inserts a tuple. Returns the slot_id if successful, or -1 if no space is available.
    int32_t insertTuple(const char* tupleData, uint16_t tupleLen);

    // Retrieves a tuple's data as a string
    std::string getTuple(slot_id_t slotId) const;

    // Deletes a tuple (lazy delete by setting offset/len to 0)
    void deleteTuple(slot_id_t slotId);

    // Gets the amount of contiguous free space in the middle of the page
    uint16_t getFreeSpace() const;

    uint16_t getSlotCount() const;

private:
    Page* page_;
    char* data_;
    
    uint16_t getFreeSpacePointer() const;
    void setFreeSpacePointer(uint16_t ptr);
    
    void setSlotCount(uint16_t count);

    Slot getSlot(slot_id_t slotId) const;
    void setSlot(slot_id_t slotId, const Slot& slot);

    static constexpr size_t FREE_SPACE_OFFSET = 0;
    static constexpr size_t SLOT_COUNT_OFFSET = 2;
    static constexpr size_t HEADER_SIZE = 4;
    static constexpr size_t SLOT_SIZE = sizeof(Slot);
};

#endif // SLOTTED_PAGE_H
