#ifndef BUFFER_MANAGER_H
#define BUFFER_MANAGER_H

#include "Constants.h"
#include "Page.h"

class BufferManager
{
  public:
    const int bufferSize;

    // Constructor that initializes bufferSize.
    BufferManager(int bufferSize) : bufferSize(bufferSize) {}

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
};

#endif // BUFFER_MANAGER_H