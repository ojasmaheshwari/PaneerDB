#ifndef BUFFER_POOL_MANAGER_H
#define BUFFER_POOL_MANAGER_H

#include "DiskManager/DiskManager.h"
#include "Eviction/ClockSweepCache.h"
#include "Page.h"

#include <list>
#include <mutex>
#include <unordered_map>
#include <vector>

using frame_id_t = int;

class BufferPoolManager {
public:
  BufferPoolManager(size_t poolSize, DiskManager *diskManager);
  ~BufferPoolManager();

  // Fetch the requested page from the buffer pool.
  Page *fetchPage(page_id_t pageId);

  // Unpin the target page from the buffer pool.
  bool unpinPage(page_id_t pageId, bool isDirty);

  // Flushes the target page to disk.
  bool flushPage(page_id_t pageId);

  // Creates a new page in the buffer pool. Set pageId to the new page's id.
  Page *newPage(page_id_t *pageId);

  // Deletes a page from the buffer pool.
  bool deletePage(page_id_t pageId);

  // Flushes all the pages in the buffer pool to disk.
  void flushAllPages();

  inline DiskManager* getDiskManager() const { return diskManager; }

private:
  size_t poolSize;
  Page *pages;
  DiskManager *diskManager;
  std::unordered_map<page_id_t, frame_id_t> pageTable;
  std::list<frame_id_t> freeList;
  ClockSweepCache<frame_id_t> replacer;
  std::mutex latch;

  // Helper method to find a free frame or evict one
  bool getVictimFrame(frame_id_t *frameId);

  // Callback from ClockSweepCache when a page is evicted by background thread
  void onEvict(int pageId, frame_id_t frameId);
};

#endif // BUFFER_POOL_MANAGER_H
