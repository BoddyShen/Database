#include "BufferManager.h"
#include "Constants.h"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <unordered_map>
#include <vector>

BufferManager::BufferManager(int bufferSize, const std::string &dbPath) : bufferSize(bufferSize)
{
    nextPageId = 0;
    // initialize buffer pool and set all the new pages to nullptr, they are empty now
    bufferPool.resize(bufferSize, nullptr);
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
        int frameIndex = findEmptyFrame();

        Page *page = new Page();
        dbData.seekg(pageId * MAX_PAGE_SIZE);
        dbData.read(reinterpret_cast<char *>(page->getPageData()), MAX_PAGE_SIZE);

        bufferPool[frameIndex] = page;

        pageTable[pageId] = frameIndex;
        pageMetadata[frameIndex].pageId = pageId;
        pageMetadata[frameIndex].isDirty = false;
        pageMetadata[frameIndex].pinCount += 1;

        updateLruQueue(frameIndex);
        return bufferPool[frameIndex];
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

    // create a new page
    Page *newPage = new Page();

    // add the new page to the buffer pool
    bufferPool[frameIndex] = newPage;

    // update the page table
    pageTable[pageId] = frameIndex;
    cout << "pageId: " << pageId << " frameIndex: " << frameIndex << endl;

    // update the metadata
    pageMetadata[frameIndex].pageId = pageId;
    pageMetadata[frameIndex].pinCount += 1;
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
    }
}

void BufferManager::unpinPage(int pageId)
{
    if (pageTable.find(pageId) != pageTable.end()) {
        if (pageMetadata[pageTable[pageId]].pinCount > 0) {
            pageMetadata[pageTable[pageId]].pinCount--;
        }
    }
}

int BufferManager::findEmptyFrame()
{
    // Find an empty frame
    Node *curr = lruCache->getFirstNode();
    while (curr != nullptr) {
        if (pageMetadata[curr->val].pageId == -1) return curr->val;
        curr = curr->next;
    }

    cout << "No empty frame found, evicting a page..." << endl;
    // If no empty frame is found, evict a page
    int frameIndex = findLRUFrame();
    if (frameIndex == -1) {
        std::cerr << "Error: No page can be evicted." << std::endl;
        return -1;
    }
    bufferPool[frameIndex] = nullptr;

    return frameIndex;
}

int BufferManager::findLRUFrame()
{
    // Find the least recently used frame using the LRUCache
    Node *curr = lruCache->getFirstNode();
    while (curr && curr->next != nullptr) {
        int frameId = curr->val;
        if (pageMetadata[frameId].pinCount == 0) {
            int pageId = pageMetadata[frameId].pageId;

            if (pageMetadata[frameId].isDirty) {
                // write to disk
                dbData.seekp(pageId * MAX_PAGE_SIZE);
                dbData.write(reinterpret_cast<const char *>(bufferPool[frameId]->getPageData()),
                             MAX_PAGE_SIZE);
                pageMetadata[frameId].isDirty = false;
            }
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
        if (bufferPool[i] != nullptr) {
            std::cout << "Page ID = " << pageMetadata[i].pageId
                      << ", Pin Count = " << pageMetadata[i].pinCount << ", "
                      << (pageMetadata[i].isDirty ? "Dirty" : "Clean") << std::endl;
            // Optionally print the first few bytes of the page data in hex.
            std::cout << "  Page Data (first 16 bytes): ";
            for (int j = 0; j < 16 && j < MAX_PAGE_SIZE; j++) {
                std::cout << std::setw(2) << std::setfill('0') << std::hex
                          << static_cast<int>(bufferPool[i]->getPageData()[j]) << " ";
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