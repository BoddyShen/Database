#include <fstream>
#include <unordered_map>
#include <list>
#include <vector>
#include "BufferManager.h"
#include "Constants.h"


BufferManager::BufferManager(int bufferSize) : bufferSize(bufferSize)
{
    nextPageId = 0;
    // initialize buffer pool and set all the new pages to nullptr, they are empty now
    bufferPool.resize(bufferSize, nullptr);
    // initialize page table with default values (header file)
    pageMetadata.resize(bufferSize);

    dbData.open(DB_FILE, std::ios::in | std::ios::out | std::ios::binary);
    if (!dbData.is_open())
    {
        // create it if not existed
        dbData.open(DB_FILE, std::ios::out | std::ios::binary);
        dbData.close();
        dbData.open(DB_FILE, std::ios::in | std::ios::out | std::ios::binary);
    }

    // assume pages in disk are always full
    dbData.seekg(0, std::ios::end);
    fileSize = dbData.tellg();
    nextPageId = fileSize / MAX_PAGE_SIZE;
} 

Page* BufferManager::getPage(int pageId) {
    // if it's in the buffer pool, return it and update the lru

    if (pageTable.find(pageId) != pageTable.end()) {
        int frameIndex = pageTable[pageId];
        pageMetadata[frameIndex].pinCount++;
        // lru thing
        return bufferPool[frameIndex];
    } else {
        // if it's not in the buffer pool, load it from disk
            // if the buffer pool is full, evict a page
            // add the new page to the buffer pool
        // return the page
        int frameIndex = findEmptyFrame(pageTable);
        

        Page* page = new Page();
        dbData.seekg(pageId * MAX_PAGE_SIZE);
        dbData.read(page->data, MAX_PAGE_SIZE);

        // move to LRU part?
        delete bufferPool[frameIndex];
        bufferPool[frameIndex] = page;
    }
}



Page* BufferManager::createPage() {
    int emptyFrameIndex = findEmptyFrame(pageTable);
    pageId = nextPageId + 1;

    // create a new page
    Page* newPage = new Page();

    // add the new page to the buffer pool
    bufferPool[emptyFrameIndex] = newPage;

    // update the page table
    pageTable[pageId] = emptyFrameIndex;

    // update the metadata
    pageMetadata[emptyFrameIndex].pageId = pageId;
    pageMetadata[emptyFrameIndex].pinCount = 1;
    pageMetadata[emptyFrameIndex].isDirty = ture;

    // LRU stuffs

    return newPage;

}

void BufferManager::markDirty(int pageId) {
    // if it's in the buffer pool
    if pageTable.find[pageId] != pageTable.end() {
        pageMetadata[pageId].isDirty = true;
    }

}


void unpinPage(int pageId) {
    if pageTable.find[pageId] != pageTable.end() {
        if pageMetadata[pageId].pinCount > 0 {
            pageMetadata[pageId].pinCount--;
        }
    }

}