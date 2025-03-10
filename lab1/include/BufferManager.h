#ifndef BUFFER_MANAGER_H
#define BUFFER_MANAGER_H

#include "Constants.h"
#include "LRUCache.h"
#include "Page.h"
#include <cstdint>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>

class BufferManager
{
  public:
    const int bufferSize;

    /**
     * Constructor that initializes the BufferManager.
     * @param bufferSize The size of the buffer pool.
     * @param dbPath The path to the database file.
     */
    BufferManager(int bufferSize, const std::string &dbPath);

    /**
     * Destructor that writes all dirty pages to disk.
     */
    ~BufferManager();

    /**
     * Fetches a page from memory if available; otherwise, loads it from disk.
     * The page is immediately pinned.
     * @param pageId The ID of the page to fetch.
     * @return Pointer to the Page object.
     */
    Page *getPage(int pageId);

    /**
     * Creates a new page.
     * The page is immediately pinned.
     * @return Pointer to the newly created Page object.
     */
    Page *createPage();

    /**
     * Marks a page as dirty, indicating it needs to be written to disk before eviction.
     * @param pageId The ID of the page to mark as dirty.
     */
    void markDirty(int pageId);

    /**
     * Unpins a page in the buffer pool, allowing it to be evicted if necessary.
     * @param pageId The ID of the page to unpin.
     */
    void unpinPage(int pageId);

    /**
     * Prints the status of the BufferManager (debug use).
     */
    void printStatus();

    /**
     * Flushes all dirty pages to disk.
     */
    void flushAll();

  private:
    int findEmptyFrame();

    int findLRUFrame();

    void updateLruQueue(int frameId);

    // store pages in a buffer pool
    std::vector<Page> bufferPool;

    // key: pageId, value: frames
    std::unordered_map<int, int> pageTable;

    struct PageMetadata {
        int pageId = -1;
        bool isDirty = false;
        int pinCount = 0;
    };

    std::vector<PageMetadata> pageMetadata;

    // LRUCache for managing the used order of frame indexes
    LRUCache *lruCache;

    std::fstream dbData;

    int nextPageId;
};

#endif // BUFFER_MANAGER_H