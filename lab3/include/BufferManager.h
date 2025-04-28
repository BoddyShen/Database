#ifndef BUFFER_MANAGER_H
#define BUFFER_MANAGER_H

#include "Constants.h"
#include "LRUCache.h"
#include "Page.h"
#include <cstdint>
#include <fstream>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

// Page<char> for dummy usage, e.g. BTree
using AnyPage = std::variant<Page<char>, Page<MovieRow>, Page<WorkedOnRow>, Page<PersonRow>>;

class BufferManager
{
  public:
    const int bufferSize;

    /**
     * Constructor that initializes the BufferManager.
     * @param bufferSize The size of the buffer pool.
     * @param dbPath The path to the database file.
     */
    BufferManager(int bufferSize);

    /**
     * Register a file in the buffer manager.
     * Open the file if it exists. Otherwise create it.
     * @param filePath The file path.
     * @return true if the file already exists, otherwise false
     */
    bool registerFile(const std::string filePath);

    /**
     * Destructor of the buffer manager.
     * Flush dirty pages and release resources.
     */
    ~BufferManager();

    /**
     * Flush all dirty pages currently in memory back to disk.
     */
    void force();

    /**
     * Fetches a page from memory if available; otherwise, loads it from disk.
     * The page is immediately pinned.
     * @param pageId The ID of the page to fetch.
     * @param filePath The file storing the page
     * @return Pointer to the Page object.
     */
    template <typename RowType> Page<RowType> *getPage(int pageId, const std::string filePath);

    /**
     * Creates a new page.
     * The page is immediately pinned.
     * @param filePath The file storing the page
     * @return Pointer to the newly created Page object.
     */
    template <typename RowType> Page<RowType> *createPage(const std::string filePath);

    /**
     * Marks a page as dirty, indicating it needs to be written to disk before eviction.
     * @param pageId The ID of the page to mark as dirty.
     * @param filePath The file storing the page
     */
    void markDirty(int pageId, const std::string filePath);

    /**
     * Unpins a page in the buffer pool, allowing it to be evicted if necessary.
     * @param pageId The ID of the page to unpin.
     * @param filePath The file storing the page
     */
    void unpinPage(int pageId, const std::string filePath);

    /**
     * Prints the status of the BufferManager (debug use).
     */
    void printStatus();

  private:
    int findEmptyFrame();

    int findLRUFrame();

    void updateLruQueue(int frameId);

    std::vector<AnyPage> bufferPool;

    struct PageMetadata {
        int pageId = -1;
        bool isDirty = false;
        int pinCount = 0;
        std::string file{};
    };

    std::vector<PageMetadata> pageMetadata;

    // LRUCache for managing the used order of frame indexes
    LRUCache *lruCache;

    struct FileHandle {
        std::fstream fs;
        int nextPageId;
        // key: pageId, value: frameId
        std::unordered_map<int, int> pageTable;
    };

    // key: filePath, value: pointer to the fileHandle
    std::unordered_map<std::string, FileHandle *> fileTable;
};

#endif // BUFFER_MANAGER_H