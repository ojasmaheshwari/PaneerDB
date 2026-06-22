#ifndef DISK_MANAGER_H
#define DISK_MANAGER_H

#include <string>
#include <fstream>
#include <cstdint>

constexpr size_t PAGE_SIZE = 4096;
using page_id_t = int32_t;

class DiskManager {
public:
    DiskManager(const std::string& file);
    ~DiskManager();

    void readPage(page_id_t pageId, char* pageData);
    void writePage(page_id_t pageId, const char* pageData);
    page_id_t allocatePage();
    inline page_id_t getNextPageId() const { return nextPageId; }

private:
    std::string fileName;
    std::fstream dbIo;
    page_id_t nextPageId;
};

#endif // DISK_MANAGER_H
