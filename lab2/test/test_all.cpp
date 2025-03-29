#include "../include/BTree.h"
#include "../include/BufferManager.h"
#include "../include/Page.h"
#include "../include/Row.h"
#include "../include/Utilities.h"
#include <iostream>
#include <fstream>
#include <cassert>
#include <vector>
#include <sstream>
#include <chrono>
#include <algorithm>

using namespace std;

// Helper function to scan Movies table and get records
vector<pair<string, Rid>> scanMoviesTable(BufferManager& bm) {
    vector<pair<string, Rid>> records;
    if (!bm.registerFile("movies.bin")) {
        cout << "Warning: movies.bin not found. Please run lab1 first to create the Movies table.\n";
        return records;
    }
    
    int pageId = 0;
    while (true) {
        Page* page = bm.getPage(pageId, "movies.bin");
        if (!page) break;
        
        for (int slotId = 0; slotId < page->getNumRecords(); slotId++) {
            Row* row = page->getRow(slotId);
            if (row) {
                try {
                    string title(reinterpret_cast<const char*>(row->title.data()), row->title.size());
                    records.push_back({title, {pageId, slotId}});
                } catch (const exception& e) {
                    cerr << "Error processing row: " << e.what() << endl;
                }
                delete row;
            }
        }
        bm.unpinPage(pageId, "movies.bin");
        pageId++;
    }
    return records;
}

// Test C1: Create index on title attribute
void test_C1_title_index() {
    cout << "\n=== Test C1: Creating index on title attribute ===\n";
    BufferManager bm(20);
    BTree<string> titleIndex("title_index.bin", &bm);
    
    // Scan Movies table and build index
    auto records = scanMoviesTable(bm);
    int count = 0;
    for (const auto& record : records) {
        titleIndex.insert(record.first, record.second);
        count++;
        if (count % 1000 == 0) {
            cout << "Inserted " << count << " records into title index\n";
        }
    }
    cout << "Title index created successfully with " << count << " records\n";
}

// Test C2: Create index on movieId attribute (with bulk insert)
void test_C2_movieId_index() {
    cout << "\n=== Test C2: Creating index on movieId attribute ===\n";
    BufferManager bm(20);
    BTree<int> movieIdIndex("movieId_index.bin", &bm);
    
    if (!bm.registerFile("movies.bin")) {
        cout << "Warning: movies.bin not found. Please run lab1 first to create the Movies table.\n";
        return;
    }
    
    vector<pair<int, Rid>> records;
    int pageId = 0;
    
    while (true) {
        Page* page = bm.getPage(pageId, "movies.bin");
        if (!page) break;
        
        for (int slotId = 0; slotId < page->getNumRecords(); slotId++) {
            Row* row = page->getRow(slotId);
            if (row) {
                try {
                    string movieIdStr(reinterpret_cast<const char*>(row->movieId.data()), row->movieId.size());
                    // Remove any non-numeric characters
                    movieIdStr.erase(
                        remove_if(movieIdStr.begin(), movieIdStr.end(), 
                                [](char c) { return !isdigit(c); }),
                        movieIdStr.end()
                    );
                    if (!movieIdStr.empty()) {
                        int movieId = stoi(movieIdStr);
                        records.push_back({movieId, {pageId, slotId}});
                    }
                } catch (const exception& e) {
                    cerr << "Error processing movieId: " << e.what() << endl;
                }
                delete row;
            }
        }
        bm.unpinPage(pageId, "movies.bin");
        pageId++;
    }
    
    if (records.empty()) {
        cout << "No valid records found in the Movies table.\n";
        return;
    }
    
    // Sort by movieId for bulk insert
    sort(records.begin(), records.end());
    
    // Bulk insert
    auto start = chrono::high_resolution_clock::now();
    movieIdIndex.bulkInsert(records);
    auto end = chrono::high_resolution_clock::now();
    cout << "MovieId index created successfully with " << records.size() << " records\n";
    cout << "Bulk insert time: " 
         << chrono::duration_cast<chrono::milliseconds>(end - start).count() 
         << "ms\n";
}

// Test C3: Point search test
void test_C3_point_search() {
    cout << "\n=== Test C3: Point search test ===\n";
    BufferManager bm(20);
    
    // Test title index
    cout << "Testing title index point search...\n";
    BTree<string> titleIndex("title_index.bin", &bm);
    string searchTitle = "The Godfather";
    auto titleResults = titleIndex.search(searchTitle);
    
    cout << "Found " << titleResults.size() << " results for title '" << searchTitle << "'\n";
    for (const auto& rid : titleResults) {
        Page* page = bm.getPage(rid.first, "movies.bin");
        Row* row = page->getRow(rid.second);
        string foundTitle(reinterpret_cast<const char*>(row->title.data()), row->title.size());
        cout << "Found movie: " << foundTitle << "\n";
        assert(foundTitle == searchTitle);
        delete row;
        bm.unpinPage(rid.first, "movies.bin");
    }
    
    // Test movieId index
    cout << "\nTesting movieId index point search...\n";
    BTree<int> movieIdIndex("movieId_index.bin", &bm);
    int searchId = 100;
    auto idResults = movieIdIndex.search(searchId);
    
    cout << "Found " << idResults.size() << " results for movieId " << searchId << "\n";
    for (const auto& rid : idResults) {
        Page* page = bm.getPage(rid.first, "movies.bin");
        Row* row = page->getRow(rid.second);
        string movieIdStr(reinterpret_cast<const char*>(row->movieId.data()), row->movieId.size());
        int foundId = stoi(movieIdStr);
        cout << "Found movie: " << string(reinterpret_cast<const char*>(row->title.data()), row->title.size())
             << " (ID: " << foundId << ")\n";
        assert(foundId == searchId);
        delete row;
        bm.unpinPage(rid.first, "movies.bin");
    }
}

// Test C4: Range search test
void test_C4_range_search() {
    cout << "\n=== Test C4: Range search test ===\n";
    BufferManager bm(20);
    
    // Test title index range search
    cout << "Testing title index range search...\n";
    BTree<string> titleIndex("title_index.bin", &bm);
    string startTitle = "The";
    string endTitle = "Thf";
    auto titleResults = titleIndex.rangeSearch(startTitle, endTitle);
    
    cout << "Found " << titleResults.size() << " results in title range [" 
         << startTitle << ", " << endTitle << "]\n";
    for (const auto& rid : titleResults) {
        Page* page = bm.getPage(rid.first, "movies.bin");
        Row* row = page->getRow(rid.second);
        string foundTitle(reinterpret_cast<const char*>(row->title.data()), row->title.size());
        cout << "Found movie: " << foundTitle << "\n";
        assert(foundTitle >= startTitle && foundTitle <= endTitle);
        delete row;
        bm.unpinPage(rid.first, "movies.bin");
    }
    
    // Test movieId index range search
    cout << "\nTesting movieId index range search...\n";
    BTree<int> movieIdIndex("movieId_index.bin", &bm);
    int startId = 100;
    int endId = 200;
    auto idResults = movieIdIndex.rangeSearch(startId, endId);
    
    cout << "Found " << idResults.size() << " results in movieId range [" 
         << startId << ", " << endId << "]\n";
    for (const auto& rid : idResults) {
        Page* page = bm.getPage(rid.first, "movies.bin");
        Row* row = page->getRow(rid.second);
        string movieIdStr(reinterpret_cast<const char*>(row->movieId.data()), row->movieId.size());
        int foundId = stoi(movieIdStr);
        cout << "Found movie: " << string(reinterpret_cast<const char*>(row->title.data()), row->title.size())
             << " (ID: " << foundId << ")\n";
        assert(foundId >= startId && foundId <= endId);
        delete row;
        bm.unpinPage(rid.first, "movies.bin");
    }
}

// Test P1: Performance comparison with title index
void test_P1_performance() {
    cout << "\n=== Test P1: Title index performance test ===\n";
    BufferManager bm(20);
    BTree<string> titleIndex("title_index.bin", &bm);
    
    string searchTitle = "The Godfather";
    
    // Measure index search time
    auto indexStart = chrono::high_resolution_clock::now();
    auto indexResults = titleIndex.search(searchTitle);
    auto indexEnd = chrono::high_resolution_clock::now();
    auto indexTime = chrono::duration_cast<chrono::microseconds>(indexEnd - indexStart).count();
    
    cout << "Index search time: " << indexTime << "µs\n";
    cout << "Found " << indexResults.size() << " results using index\n";
    
    // Measure table scan time
    auto scanStart = chrono::high_resolution_clock::now();
    vector<Rid> scanResults;
    int pageId = 0;
    int maxPages = 1000; // Limit scan to prevent infinite loop
    
    while (pageId < maxPages) {
        Page* page = bm.getPage(pageId, "movies.bin");
        if (!page) break;
        
        for (int slotId = 0; slotId < page->getNumRecords(); slotId++) {
            Row* row = page->getRow(slotId);
            if (row) {
                string title(reinterpret_cast<const char*>(row->title.data()), row->title.size());
                if (title == searchTitle) {
                    scanResults.push_back({pageId, slotId});
                }
                delete row;
            }
        }
        bm.unpinPage(pageId, "movies.bin");
        pageId++;
    }
    
    auto scanEnd = chrono::high_resolution_clock::now();
    auto scanTime = chrono::duration_cast<chrono::microseconds>(scanEnd - scanStart).count();
    
    cout << "Table scan time: " << scanTime << "µs\n";
    cout << "Found " << scanResults.size() << " results using table scan\n";
    cout << "Speed-up: " << (double)scanTime / indexTime << "x\n";
}

// Test P2: Performance comparison with movieId index
void test_P2_performance() {
    cout << "\n=== Test P2: MovieId index performance test ===\n";
    BufferManager bm(20);
    BTree<int> movieIdIndex("movieId_index.bin", &bm);
    
    int searchId = 100;
    
    // Measure index search time
    auto indexStart = chrono::high_resolution_clock::now();
    auto indexResults = movieIdIndex.search(searchId);
    auto indexEnd = chrono::high_resolution_clock::now();
    auto indexTime = chrono::duration_cast<chrono::microseconds>(indexEnd - indexStart).count();
    
    cout << "Index search time: " << indexTime << "µs\n";
    cout << "Found " << indexResults.size() << " results using index\n";
    
    // Measure table scan time
    auto scanStart = chrono::high_resolution_clock::now();
    vector<Rid> scanResults;
    int pageId = 0;
    int maxPages = 1000; // Limit scan to prevent infinite loop
    
    while (pageId < maxPages) {
        Page* page = bm.getPage(pageId, "movies.bin");
        if (!page) break;
        
        for (int slotId = 0; slotId < page->getNumRecords(); slotId++) {
            Row* row = page->getRow(slotId);
            if (row) {
                try {
                    string movieIdStr(reinterpret_cast<const char*>(row->movieId.data()), row->movieId.size());
                    int movieId = stoi(movieIdStr);
                    if (movieId == searchId) {
                        scanResults.push_back({pageId, slotId});
                    }
                } catch (const exception& e) {
                    // Skip invalid movieIds
                }
                delete row;
            }
        }
        bm.unpinPage(pageId, "movies.bin");
        pageId++;
    }
    
    auto scanEnd = chrono::high_resolution_clock::now();
    auto scanTime = chrono::duration_cast<chrono::microseconds>(scanEnd - scanStart).count();
    
    cout << "Table scan time: " << scanTime << "µs\n";
    cout << "Found " << scanResults.size() << " results using table scan\n";
    cout << "Speed-up: " << (double)scanTime / indexTime << "x\n";
}

int main() {
    try {
        // First check if movies.bin exists
        ifstream f("movies.bin");
        if (!f.good()) {
            cout << "Error: movies.bin not found. Please run lab1 first to create the Movies table.\n";
            return 1;
        }
        
        // Correctness Tests
        test_C1_title_index();
        test_C2_movieId_index();
        test_C3_point_search();
        test_C4_range_search();
        
        // Performance Tests
        test_P1_performance();
        test_P2_performance();
        
        cout << "\nAll tests completed successfully!\n";
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    return 0;
}