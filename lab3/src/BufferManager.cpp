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

using namespace std;

BufferManager::BufferManager(int bufferSize) : bufferSize(bufferSize)
{
    // initialize buffer pool
    bufferPool.resize(bufferSize);
    // initialize page table with default values (header file)
    pageMetadata.resize(bufferSize);
    // initialize LRU double linked list with fixed bufferSize
    lruCache = new LRUCache(bufferSize);
    for (int i = 0; i < bufferSize; i++) {
        lruCache->put(i);
    }
}

bool BufferManager::registerFile(const std::string filePath)
{
    FileHandle *handle = new FileHandle();
    std::fstream &fs = handle->fs;
    fs.open(filePath, std::ios::in | std::ios::out | std::ios::binary);
    bool is_existed = true;
    if (!fs.is_open()) {
        is_existed = false;
        // create it if not existed
        fs.open(filePath, std::ios::out | std::ios::binary);
        fs.close();
        fs.open(filePath, std::ios::in | std::ios::out | std::ios::binary);
    }

    // assume pages in disk are always full
    fs.seekg(0, std::ios::end);
    std::streampos fileSize = fs.tellg();
    // std::cout << "File size: " << fileSize << std::endl;
    int nextPageId = fileSize / MAX_PAGE_SIZE;
    handle->nextPageId = nextPageId;
    // std::cout << "nextPageId: " << nextPageId << std::endl;
    fileTable[filePath] = handle;
    std::cout << "file " << filePath << " registered\n";
    return is_existed;
}

BufferManager::~BufferManager()
{
    force();
    delete lruCache;
    for (auto [key, value] : fileTable) {
        delete value;
    }
}

void BufferManager::force()
{
    cout << "force all pages to disk" << endl;
    // write all dirty pages to disk
    for (int i = 0; i < bufferSize; i++) {
        if (pageMetadata[i].pinCount != 0) {
            std::cerr << "Error: page " << pageMetadata[i].pageId
                      << " is still pinned, cannot force to disk!" << std::endl;
            std::cerr << "file: " << pageMetadata[i].file << endl;
            continue;
        }
        assert(pageMetadata[i].pinCount == 0);
        if (pageMetadata[i].isDirty) {
            int pageId = pageMetadata[i].pageId;
            fstream &fs = fileTable[pageMetadata[i].file]->fs;
            fs.seekp(pageId * MAX_PAGE_SIZE);
            // write back whichever Page<T> is actually in bufferPool[frameId]
            std::visit(
                [&](auto &pg) {
                    fs.write(reinterpret_cast<const char *>(pg.getPageData()), MAX_PAGE_SIZE);
                },
                bufferPool[i]);
            fs.flush();
            if (fs.fail()) {
                std::cerr << "Error writing page " << pageId << " to disk!" << std::endl;
                exit(0);
            }
        }
    }
    cout << "force success" << endl;
}

template <typename RowType>
Page<RowType> *BufferManager::getPage(int pageId, const std::string filePath)
{
    // if the file is not registered, return nullptr
    if (fileTable.find(filePath) == fileTable.end()) {
        std::cerr << "Error: No such filePath " << filePath << "." << std::endl;
        return nullptr;
    }

    int frameIndex;

    auto &pageTable = fileTable[filePath]->pageTable;
    // if it's in the buffer pool, return the pointer to it and update the lru
    if (pageTable.find(pageId) != pageTable.end()) {
        frameIndex = pageTable[pageId];
        assert(pageMetadata[frameIndex].pageId == pageId);
        pageMetadata[frameIndex].pinCount++;
        updateLruQueue(frameIndex);
    } else {
        // if it's not in the buffer pool, load it from disk
        frameIndex = findEmptyFrame();

        // if the buffer pool is full and all pages are pinned, return nullptr
        if (frameIndex == -1) {
            std::cerr << "all pages are pinned, getPage failed\n";
            return nullptr;
        }

        // load the requested page from disk
        // we assume the disk file contains the page, the pid check logic should be implemented by
        // the caller, pageId logic may differ in data file and index file
        bufferPool[frameIndex].template emplace<Page<RowType>>();
        auto &typedPage = std::get<Page<RowType>>(bufferPool[frameIndex]);
        // auto &typedPage = get<Page<RowType>>(bufferPool[frameIndex]);
        typedPage.setPid(pageId);
        fileTable[filePath]->fs.seekg(pageId * MAX_PAGE_SIZE);
        fileTable[filePath]->fs.read(reinterpret_cast<char *>(typedPage.getPageData()),
                                     MAX_PAGE_SIZE);
        int num;
        std::memcpy(&num, typedPage.getPageData(), sizeof(num));
        typedPage.setNumRecords(num);

        pageTable[pageId] = frameIndex;
        pageMetadata[frameIndex].pageId = pageId;
        pageMetadata[frameIndex].isDirty = false;
        pageMetadata[frameIndex].pinCount = 1;
        pageMetadata[frameIndex].file = filePath;

        updateLruQueue(frameIndex);
    }

    auto &page = get<Page<RowType>>(bufferPool[frameIndex]);
    return &page;
}

template <typename RowType> Page<RowType> *BufferManager::createPage(const std::string filePath)
{
    // if the file is not registered, return nullptr
    if (fileTable.find(filePath) == fileTable.end()) {
        std::cerr << "Error: No such filePath " << filePath << "." << std::endl;
        return nullptr;
    }

    auto &pageTable = fileTable[filePath]->pageTable;
    int frameIndex = findEmptyFrame();
    if (frameIndex == -1) {
        std::cerr << "Error: No empty frame found." << std::endl;
        return nullptr;
    }
    int pageId = fileTable[filePath]->nextPageId++;

    // add the new page to the buffer pool
    bufferPool[frameIndex] = AnyPage{std::in_place_type<Page<RowType>>};
    auto &newPage = std::get<Page<RowType>>(bufferPool[frameIndex]);

    // update the page table and page itself
    pageTable[pageId] = frameIndex;
    newPage.setPid(pageId);

    // update the metadata
    pageMetadata[frameIndex].pageId = pageId;
    pageMetadata[frameIndex].pinCount = 1;
    pageMetadata[frameIndex].isDirty = true;
    pageMetadata[frameIndex].file = filePath;

    // LRU stuffs
    updateLruQueue(frameIndex);
    return &newPage;
}

void BufferManager::markDirty(int pageId, const std::string filePath)
{
    if (fileTable.find(filePath) == fileTable.end()) {
        std::cout << "file " << filePath << " not registered!" << std::endl;
    }
    auto &pageTable = fileTable[filePath]->pageTable;
    // if it's in the buffer pool
    if (pageTable.find(pageId) != pageTable.end()) {
        pageMetadata[pageTable[pageId]].isDirty = true;
    } else {
        cerr << "page " << pageId << " not found in buffer pool!" << endl;
    }
}

void BufferManager::unpinPage(int pageId, const std::string filePath)
{
    if (fileTable.find(filePath) == fileTable.end()) {
        std::cout << "file " << filePath << " not registered!" << std::endl;
    }
    auto &pageTable = fileTable[filePath]->pageTable;
    if (pageTable.find(pageId) != pageTable.end()) {
        if (pageMetadata[pageTable[pageId]].pinCount > 0) {
            pageMetadata[pageTable[pageId]].pinCount--;
        } else {
            cerr << "page pinCount is already 0!" << endl;
        }
    } else {
        cerr << "page " << pageId << " not found in buffer pool!" << endl;
    }
}

int BufferManager::findEmptyFrame()
{
    // Find an empty frame
    for (int i = 0; i < bufferSize; i++) {
        if (pageMetadata[i].pageId == -1) return i;
    }
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
        if (pageMetadata[frameId].pinCount == 0) {
            int pageId = pageMetadata[frameId].pageId;
            if (pageMetadata[frameId].isDirty) {
                // write to disk
                fstream &fs = fileTable[pageMetadata[frameId].file]->fs;
                fs.seekp(pageId * MAX_PAGE_SIZE);
                // write back whichever Page<T> is actually in bufferPool[frameId]
                std::visit(
                    [&](auto &pg) {
                        fs.write(reinterpret_cast<const char *>(pg.getPageData()), MAX_PAGE_SIZE);
                    },
                    bufferPool[frameId]);
                fs.flush();
                if (fs.fail()) {
                    cerr << "Error writing page " << pageId << " to disk!" << endl;
                    exit(0);
                }

                pageMetadata[frameId].isDirty = false;
            }
            // remove from LRU cache
            lruCache->remove(frameId);
            // remove from page table
            fileTable[pageMetadata[frameId].file]->pageTable.erase(pageId);

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
    cout << "Page table content:" << endl;
    for (auto &[key, value] : fileTable) {
        cout << "file " << key << ":" << endl;
        for (auto a : value->pageTable) {
            cout << "pageId: " << a.first << " frameId: " << a.second << endl;
        }
    }
    for (size_t i = 0; i < bufferPool.size(); i++) {
        std::cout << "Frame " << i << ": ";
        if (pageMetadata[i].pageId != -1) {
            std::cout << "Page ID = " << pageMetadata[i].pageId
                      << ", Pin Count = " << pageMetadata[i].pinCount << ", "
                      << (pageMetadata[i].isDirty ? "Dirty" : "Clean") << std::endl;
            // Optionally print the first few bytes of the page data in hex.
            std::cout << "  Page Data (first 16 bytes): ";
            for (int j = 0; j < 16 && j < MAX_PAGE_SIZE; j++) {
                // Page<WorkedOnRow> &page = get<Page<WorkedOnRow>>(bufferPool[j]);
                // std::cout << std::setw(2) << std::setfill('0') << std::hex
                //           << static_cast<int>(page.getPageData()[j]) << " ";
            }
            std::cout << std::dec << std::setfill(' ') << std::endl;
        } else {
            std::cout << "Empty" << std::endl;
        }
    }

    std::cout << "----- LRU Cache Content -----" << std::endl;
    cout << "LRU cache size: " << lruCache->getSize() << endl;
    // Iterate through the LRUCache linked list
    Node *curr = lruCache->getFirstNode();
    while (curr && curr->next != nullptr) { // Skip the dummy tail node
        std::cout << curr->val << " ";
        curr = curr->next;
    }
    std::cout << std::endl;
    std::cout << "==============================" << std::endl;
}

template Page<MovieRow> *BufferManager::getPage<MovieRow>(int, std::string);
template Page<WorkedOnRow> *BufferManager::getPage<WorkedOnRow>(int, std::string);
template Page<PersonRow> *BufferManager::getPage<PersonRow>(int, std::string);

template Page<MovieRow> *BufferManager::createPage<MovieRow>(std::string);
template Page<WorkedOnRow> *BufferManager::createPage<WorkedOnRow>(std::string);
template Page<PersonRow> *BufferManager::createPage<PersonRow>(std::string);

// for dummy usage, e.g. BTree
template Page<char> *BufferManager::getPage<char>(int, std::string);
template Page<char> *BufferManager::createPage<char>(std::string);

template Page<WorkedOnKeyRow> *BufferManager::getPage<WorkedOnKeyRow>(int, std::string);
template Page<WorkedOnKeyRow> *BufferManager::createPage<WorkedOnKeyRow>(std::string);

template Page<MovieWorkedOnRow> *BufferManager::getPage<MovieWorkedOnRow>(int, std::string);
template Page<MovieWorkedOnRow> *BufferManager::createPage<MovieWorkedOnRow>(std::string);