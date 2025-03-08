#include "BufferManager.h"
#include "Constants.h"
#include "Utilities.h"
#include <cassert>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <string>
#include <unordered_map>
#include <vector>

BufferManager::BufferManager(int bufferSize, const std::string &dbPath) : bufferSize(bufferSize)
{
    nextPageId = 0;
    // initialize buffer pool
    bufferPool.resize(bufferSize);
    // initialize page table with default values (header file)
    pageMetadata.resize(bufferSize);
    // initialize LRU double linked list with fixed bufferSize
    lruCache = new LRUCache(bufferSize);
    for (int i = 0; i < bufferSize; i++) {
        lruCache->put(i);
    }

    dbData.open(dbPath, std::ios::in | std::ios::out | std::ios::binary);
    if (!dbData.is_open()) {
        // create it if not existed
        dbData.open(dbPath, std::ios::out | std::ios::binary);
        dbData.close();
        dbData.open(dbPath, std::ios::in | std::ios::out | std::ios::binary);
    }

    // assume pages in disk are always full
    dbData.seekg(0, std::ios::end);
    std::streampos fileSize = dbData.tellg();
    std::cout << "File size: " << fileSize << std::endl;
    nextPageId = fileSize / MAX_PAGE_SIZE;
    std::cout << "nextPageId: " << nextPageId << std::endl;
}

Page *BufferManager::getPage(int pageId)
{
    // if it's in the buffer pool, return the pointer to it and update the lru
    if (pageTable.find(pageId) != pageTable.end()) {
        int frameIndex = pageTable[pageId];
        assert(pageMetadata[frameIndex].pageId == pageId);
        pageMetadata[frameIndex].pinCount++;
        updateLruQueue(frameIndex);
        return &bufferPool[frameIndex];
    } else {
        // if it's not in the buffer pool, load it from disk
        int frameIndex = findEmptyFrame();

        // if the buffer pool is full and all pages are pinned, return nullptr
        if (frameIndex == -1) return nullptr;

        // load the requested page from disk
        // we assume the disk file contains the page, the pid check logic should be implemented by
        // the caller
        Page *page = &bufferPool[frameIndex];
        page->setPid(pageId);
        dbData.seekg(pageId * MAX_PAGE_SIZE);
        dbData.read(reinterpret_cast<char *>(page->getPageData()), MAX_PAGE_SIZE);
        int num;
        std::memcpy(&num, page->getPageData(), sizeof(num));
        page->setNumRecords(num);

        pageTable[pageId] = frameIndex;
        pageMetadata[frameIndex].pageId = pageId;
        pageMetadata[frameIndex].isDirty = false;
        pageMetadata[frameIndex].pinCount = 1;

        updateLruQueue(frameIndex);
        return page;
    }
}

Page *BufferManager::createPage()
{

    int frameIndex = findEmptyFrame();
    if (frameIndex == -1) {
        std::cerr << "Error: No empty frame found." << std::endl;
        return nullptr;
    }
    int pageId = nextPageId++;

    // add the new page to the buffer pool
    bufferPool[frameIndex] = Page();
    Page *newPage = &bufferPool[frameIndex];

    // update the page table and page itself
    pageTable[pageId] = frameIndex;
    newPage->setPid(pageId);

    // update the metadata
    pageMetadata[frameIndex].pageId = pageId;
    pageMetadata[frameIndex].pinCount = 1;
    pageMetadata[frameIndex].isDirty = true;

    // LRU stuffs
    updateLruQueue(frameIndex);
    return newPage;
}

void BufferManager::markDirty(int pageId)
{
    // if it's in the buffer pool
    if (pageTable.find(pageId) != pageTable.end()) {
        pageMetadata[pageTable[pageId]].isDirty = true;
    } else {
        cerr << "page not found in buffer pool!" << endl;
        exit(0);
    }
}

void BufferManager::unpinPage(int pageId)
{
    if (pageTable.find(pageId) != pageTable.end()) {
        if (pageMetadata[pageTable[pageId]].pinCount > 0) {
            pageMetadata[pageTable[pageId]].pinCount--;
        } else {
            cerr << "page pinCoint is already 0!" << endl;
            exit(0);
        }
    } else {
        cerr << "page not found in buffer pool!" << endl;
        exit(0);
    }
}

int BufferManager::findEmptyFrame()
{
    // Find an empty frame
    for (int i = 0; i < bufferSize; i++) {
        if (pageMetadata[i].pageId == -1) return i;
    }

    cout << "No empty frame found, evicting a page..." << endl;
    // If no empty frame is found, evict a page
    int frameIndex = findLRUFrame();
    if (frameIndex == -1) {
        std::cerr << "Error: No page can be evicted." << std::endl;
        return -1;
    }

    return frameIndex;
}

int BufferManager::findLRUFrame()
{
    assert(lruCache->getSize() == bufferSize);
    // Find the least recently used frame using the LRUCache
    Node *curr = lruCache->getFirstNode();
    while (curr && curr->next != nullptr) {
        int frameId = curr->val;
        cout << "Checking frame " << frameId << "..." << endl;

        if (pageMetadata[frameId].pinCount == 0) {
            cout << "Evicting frame " << frameId << " (page id:" << pageMetadata[frameId].pageId
                 << ")..." << endl;
            int pageId = pageMetadata[frameId].pageId;

            if (pageMetadata[frameId].isDirty) {
                // write to disk
                cout << "Writing page " << pageId << " to disk..." << endl;
                Page *evictedPage = &bufferPool[frameId];
                dbData.seekp(pageId * MAX_PAGE_SIZE);
                dbData.write(reinterpret_cast<const char *>(evictedPage->getPageData()),
                             MAX_PAGE_SIZE);
                dbData.flush();
                if (dbData.fail()) {
                    cerr << "Error writing page " << pageId << " to disk!" << endl;
                    exit(0);
                }

                pageMetadata[frameId].isDirty = false;
            }
            // remove from LRU cache
            lruCache->remove(frameId);
            // remove from page table
            pageTable.erase(pageId);

            return frameId;
        }
        curr = curr->next;
    };

    return -1;
}

void BufferManager::updateLruQueue(int frameId) { lruCache->put(frameId); }

// Print the status of the BufferManager: buffer pool and LRUCache contents.
void BufferManager::printStatus()
{
    std::cout << "===== Buffer Pool Status =====" << std::endl;
    for (size_t i = 0; i < bufferPool.size(); i++) {
        std::cout << "Frame " << i << ": ";
        if (pageMetadata[i].pageId != -1) {
            std::cout << "Page ID = " << pageMetadata[i].pageId
                      << ", Pin Count = " << pageMetadata[i].pinCount << ", "
                      << (pageMetadata[i].isDirty ? "Dirty" : "Clean") << std::endl;
            // Optionally print the first few bytes of the page data in hex.
            std::cout << "  Page Data (first 16 bytes): ";
            for (int j = 0; j < 16 && j < MAX_PAGE_SIZE; j++) {
                std::cout << std::setw(2) << std::setfill('0') << std::hex
                          << static_cast<int>(bufferPool[i].getPageData()[j]) << " ";
            }
            std::cout << std::dec << std::setfill(' ') << std::endl;
        } else {
            std::cout << "Empty" << std::endl;
        }
    }

    std::cout << "----- LRU Cache Content -----" << std::endl;
    // Iterate through the LRUCache linked list
    Node *curr = lruCache->getFirstNode();
    while (curr && curr->next != nullptr) { // Skip the dummy tail node
        std::cout << curr->val << " ";
        curr = curr->next;
    }
    std::cout << std::endl;
    std::cout << "==============================" << std::endl;
}