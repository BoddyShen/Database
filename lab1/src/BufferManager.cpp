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
        updateLruQueue(frameIndex);
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


        pageTable[pageId] = frameIndex;
        pageMetadata[frameId].pageId = pageId;
        pageMetadata[frameId].isDirty = false;
        pageMetadata[frameId].pinCount += 1;

        updateLruQueue(frameId);
        return bufferPool[frameId];
    }
}



Page* BufferManager::createPage() {
    int frameIndex = findEmptyFrame(pageTable);
    int pageId = nextPageId + 1;

    // create a new page
    Page* newPage = new Page();

    // add the new page to the buffer pool
    bufferPool[frameIndex] = newPage;

    // update the page table
    pageTable[pageId] = frameIndex;

    // update the metadata
    pageMetadata[frameIndex].pageId = pageId;
    pageMetadata[frameIndex].pinCount = 1;
    pageMetadata[frameIndex].isDirty = ture;

    // LRU stuffs
    updateLruQueue(frameIndex);
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

int BufferManager::findEmptyFrame() {
    // find an empty frame
    int frameIndex = -1;
    for (int i = 0; i < pageTable.size(); i++) {
        if (pageTable[i].pageId == -1) {
            frameIndex = pageTable[i];
        }
    }

    frameIndex = findLRUFrame(pageTable);

    delete bufferPool[frameIndex];
    bufferPool[frameIndex] = page;

    return frameIndex;
}

int BufferManager::findLRUFrame() {
    // find the least recently used frame
    for (int i = lruQueue.size() - 1; i >=0; i--) {
        int frameId = lruQueue[i];
        if (PageMetadata[frameId].pinCount == 0) {
            int pageId = PageMetadata[frameId].pageId;

            if (PageMetadata[frameId].isDirty) {
                // write to disk
                dbData.seekp(pageId * MAX_PAGE_SIZE);
                dbData.write(bufferPool[frameId]->data, MAX_PAGE_SIZE);

                pageMetadata[frameId].isDirty = false;
            }
            // remove from page table
            pageTable.erase(pageId);
            // remove from lru queue
            lruQueue.erase(lruQueue.begin() + i);
            // remove from lru map
            lruMap.erase(frameId);

            return frameId;
        }
    };
}

void BufferManager::updateLruQueue(int frameId) {
    // if frameId exist in the map then erase
    auto index = lruMap.find[frameId];
    if (index != lruMap.end()) {
        lruQueue.erase(lruQueue.begin() + index);
    }
    
    // move or add the frame to the front
    lruQueue.push_front(frameId);
    lruMap[frameId] = lruQueue.begin();
}