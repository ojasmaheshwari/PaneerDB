#include "DiskManager.h"
#include <stdexcept>
#include <cstring>
#include <iostream>

DiskManager::DiskManager(const std::string& file) : fileName(file), nextPageId(0) {
    dbIo.open(file, std::ios::binary | std::ios::in | std::ios::out);

    // If file doesn't exist, create it
    if (!dbIo.is_open()) {
        dbIo.clear();
        dbIo.open(file, std::ios::binary | std::ios::trunc | std::ios::out);
        dbIo.close();
        dbIo.open(file, std::ios::binary | std::ios::in | std::ios::out);
        if (!dbIo.is_open()) {
            throw std::runtime_error("can't open db file");
        }
    }

    // Determine the next page ID based on the file size
    dbIo.seekg(0, std::ios::end);
    std::streampos size = dbIo.tellg();
    nextPageId = size / PAGE_SIZE;
}

DiskManager::~DiskManager() {
    if (dbIo.is_open()) {
        dbIo.close();
    }
}

void DiskManager::readPage(page_id_t pageId, char* pageData) {
    int offset = pageId * PAGE_SIZE;
    if (pageId >= nextPageId) {
        throw std::runtime_error("Reading past end of file");
    }

    dbIo.seekg(offset);
    dbIo.read(pageData, PAGE_SIZE);
    
    if (dbIo.bad()) {
        throw std::runtime_error("I/O error while reading");
    }
    
    int readCount = dbIo.gcount();
    if (readCount < PAGE_SIZE) {
        // Fill the rest with 0s if we read a partial page
        dbIo.clear();
        std::memset(pageData + readCount, 0, PAGE_SIZE - readCount);
    }
}

void DiskManager::writePage(page_id_t pageId, const char* pageData) {
    int offset = pageId * PAGE_SIZE;
    dbIo.seekp(offset);
    dbIo.write(pageData, PAGE_SIZE);
    
    if (dbIo.bad()) {
        throw std::runtime_error("I/O error while writing");
    }
    dbIo.flush();
}

page_id_t DiskManager::allocatePage() {
    page_id_t id = nextPageId++;
    return id;
}
