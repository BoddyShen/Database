#include "../include/BTree.h"
#include "../include/BufferManager.h"
#include "../include/Constants.h"
#include "../include/DatabaseCatalog.h"
#include "../include/Page.h"
#include "../include/Row.h"
#include "../include/Utilities.h"
#include <algorithm>
#include <cassert>
#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <vector>

using namespace std;

// This test is for a small dataset (2000 rows) in title.basics to demonstrate the correctness and
// performance of the

// Load movie data (title.basics.2000.tsv) into the database
// title.basics.2000.tsv is a subset of the IMDB dataset containing 2000 rows.
int loadMovieData()
{
    // Remove existing database file to start with a clean slate.
    remove("movie2000.bin");
    BufferManager bm(FRAME_SIZE);
    bm.registerFile("movie2000.bin");

    // Open the TSV file
    ifstream tsvFile("../title.basics.2000.tsv");
    if (!tsvFile.is_open()) {
        cerr << "Failed to open title.basics.2000.tsv" << endl;
        return 0;
    }

    string header;
    getline(tsvFile, header);
    cout << "Header: " << header << endl;

    // Create the first append page; all data will be inserted into this page.
    Page *appendPage = bm.createPage("movie2000.bin");
    int appendPid = appendPage->getPid();
    cout << "Initial append page id: " << appendPid << endl;

    // Read the TSV file line by line.
    string line;
    int loadedRows = 0;
    while (getline(tsvFile, line)) {
        loadedRows++;
        // Parse the line using tab as a delimiter.
        istringstream iss(line);
        vector<string> tokens;
        string token;
        while (getline(iss, token, '\t')) {
            tokens.push_back(token);
        }

        if (tokens.size() < 3) continue;
        string movieId = tokens[0];
        string title = tokens[2];

        // Truncate data to fixed length: movieId to 9 characters, title to 30 characters.
        if (movieId.size() > 9) movieId = movieId.substr(0, 9);
        if (title.size() > 30) title = title.substr(0, 30);

        // Create a Row object, assuming Row has a constructor accepting C-string.
        Row row(movieId.c_str(), title.c_str());

        // If the current page is full, unpin it and create a new page.
        if (appendPage->isFull()) {
            bm.unpinPage(appendPid, "movie2000.bin");
            appendPage = bm.createPage("movie2000.bin");
            appendPid = appendPage->getPid();
            cout << "Loaded " << loadedRows << " rows" << endl;
            cout << "Created new append page, id: " << appendPid << endl;
        }

        // Insert the row into the current append page.
        int rowId = appendPage->insertRow(row);
        if (rowId == -1) {
            cerr << "Failed to insert row into page " << appendPid << endl;
        }
    }

    tsvFile.close();

    // After loading, unpin the last append page.
    bm.unpinPage(appendPid, "movie2000.bin");

    cout << "Loaded " << loadedRows << " rows into the Movies table." << endl;
    return appendPid;
}

// A generic helper to scan the Movies table.
// FixedStringType is either FixedTitleSizeString or FixedMovieIdString, since the BTrees accept
// only fixed-size keys.
template <typename FixedStringType>
vector<pair<FixedStringType, Rid>> scanMovies(BufferManager &bm, const std::string &movieFile,
                                              std::function<string(const Row &)> keyExtractor)
{
    // Check if the movies file exists; if not, load movie data.
    ifstream f(movieFile, ios::binary);
    if (!f.good()) {
        cout << movieFile << " not found. Loading movie data...\n";
        loadMovieData();
    }
    // Get the actual file size.
    f.seekg(0, ios::end);
    std::streampos fileSize = f.tellg();
    f.close();

    vector<pair<FixedStringType, Rid>> records;
    bm.registerFile(movieFile);

    int pageId = 0;
    while (pageId * MAX_PAGE_SIZE < fileSize) {
        Page *page = bm.getPage(pageId, movieFile);
        if (!page) break;
        for (int slotId = 0; slotId < page->getNumRecords(); slotId++) {
            Row *row = page->getRow(slotId);
            if (row) {
                try {
                    // Use the provided keyExtractor to get the key.
                    string key = keyExtractor(*row);
                    records.push_back({FixedStringType(key), {pageId, slotId}});
                } catch (const exception &e) {
                    cerr << "Error processing row: " << e.what() << endl;
                }
                delete row;
            }
        }
        bm.unpinPage(pageId, movieFile);
        if (pageId % 10000 == 0) cout << "Scanned page " << pageId << endl;
        pageId++;
    }
    cout << "Scanned " << pageId << " pages from the Movies table\n";
    cout << "Scanned " << records.size() << " records from the Movies table\n";
    return records;
}

// Test C1: Create index on title attribute
void test_C1_title_index(DatabaseCatalog &catalog)
{
    cout << "\n=== Test C1: Creating index on title attribute ===\n";

    string indexName = "title_index2000.bin";
    ifstream f(indexName, ios::binary);
    bool indexExists = f.good();
    f.close();

    if (indexExists) {
        cout << "Title index already exists. Skipping creation.\n";
    } else {
        BufferManager bm(20);
        BTree<FixedTitleSizeString> titleIndex(indexName, &bm);

        // Scan Movies table and build index using a lambda to extract the title.
        vector<pair<FixedTitleSizeString, Rid>> titleRecords =
            scanMovies<FixedTitleSizeString>(bm, "movie2000.bin", [](const Row &row) -> string {
                return string(reinterpret_cast<const char *>(row.title.data()), row.title.size());
            });

        int count = 0;
        for (const auto &record : titleRecords) {
            titleIndex.insert(record.first, record.second);
            count++;
            if (count % 100000 == 0) {
                cout << "Inserted " << count << " records into title index\n";
                cout << "Last record: " << record.first.toString() << "\n";
                cout << "Last record RID: " << record.second.first << ", " << record.second.second
                     << "\n";
            }
        }
        cout << "Title index created successfully with " << count << " records\n";
    }

    // Add the index information into the catalog once, regardless of index file existence.
    DatabaseCatalog::IndexInfo titleIndexInfo;
    titleIndexInfo.indexName = indexName;
    titleIndexInfo.tableName = "Movie";  // Name of the Movies table
    titleIndexInfo.filePath = indexName; // File path for the index
    titleIndexInfo.keyName = "title";    // The search attribute
    catalog.addIndex(titleIndexInfo);
    cout << "Added title index information to the catalog\n";
}

// Test C2: Create index on movieId attribute (with bulk insert)
void test_C2_movieId_index(DatabaseCatalog &catalog)
{
    cout << "\n=== Test C2: Creating index on movieId attribute ===\n";

    string indexName = "movieId_index2000.bin";
    ifstream f(indexName, ios::binary);
    bool indexExists = f.good();
    f.close();

    if (indexExists) {
        cout << "MovieId index already exists. Skipping creation.\n";
    } else {
        BufferManager bm(20);
        BTree<FixedMovieIdString> movieIdIndex(indexName, &bm);

        // Scan Movies table and build index
        vector<pair<FixedMovieIdString, Rid>> movieIdRecords =
            scanMovies<FixedMovieIdString>(bm, "movie2000.bin", [](const Row &row) -> string {
                return string(reinterpret_cast<const char *>(row.movieId.data()),
                              row.movieId.size());
            });

        // Bulk insert
        cout << "Bulk inserting movieId index...\n";
        auto start = chrono::high_resolution_clock::now();
        movieIdIndex.bulkInsert(movieIdRecords);
        auto end = chrono::high_resolution_clock::now();
        cout << "MovieId index created successfully with " << movieIdRecords.size() << " records\n";
        cout << "Bulk insert time: "
             << chrono::duration_cast<chrono::milliseconds>(end - start).count() << "ms\n";
    }

    // Add the index information into the catalog.
    DatabaseCatalog::IndexInfo titleIndexInfo;
    titleIndexInfo.indexName = indexName;
    titleIndexInfo.tableName = "Movie";
    titleIndexInfo.filePath = indexName;
    titleIndexInfo.keyName = "movieId";
    catalog.addIndex(titleIndexInfo);

    cout << "Added movieId index information to the catalog\n";
}

// Test C3: Point search test
void test_C3_point_search(DatabaseCatalog &catalog)
{
    cout << "\n=== Test C3: Point search test ===\n";
    BufferManager bm(20);

    auto movieTableInfo = catalog.getTable("Movie");
    bm.registerFile(movieTableInfo.filePath);

    // Test title index
    cout << "Testing title index point search...\n";
    vector<string> testTitles = {"The Two Reformations", "Carmencita", "Leaving the Factory"};

    auto titleIndexInfo = catalog.getIndex("title_index2000.bin");
    BTree<FixedTitleSizeString> titleIndex("title_index2000.bin", &bm);

    for (const string &searchTitle : testTitles) {
        FixedTitleSizeString searchTitleStr(searchTitle);

        cout << "Searching for movie with title: " << searchTitle << endl;
        auto titleResults = titleIndex.search(searchTitleStr);
        cout << "Found " << titleResults.size() << " results for title '" << searchTitle << "'\n";
        for (const auto &rid : titleResults) {
            int pageId = rid.first;
            int slotId = rid.second;

            Page *page = bm.getPage(pageId, movieTableInfo.filePath);
            if (!page) continue;
            Row *row = page->getRow(slotId);
            if (!row) continue;

            string foundTitle(reinterpret_cast<const char *>(row->title.data()), row->title.size());
            cout << "Found movie: " << foundTitle << "\n";
            FixedTitleSizeString foundTitleStr(foundTitle);
            assert(foundTitleStr == searchTitleStr);
            delete row;
            bm.unpinPage(pageId, movieTableInfo.filePath);
        }
    }

    // Test movieId index
    cout << "Testing movieId index point search...\n";

    unordered_map<string, string> testMovieId = {
        {"tt0000001", "Carmencita"},
        {"tt0000002", "Le clown et ses chiens"},
        {"tt0000003", "Poor Pierrot"},
        {"tt0000004", "Un bon bock"},
        {"tt0000005", "Blacksmith Scene"},
        {"tt0000006", "Chinese Opium Den"},
        {"tt0000007", "Corbett and Courtney Before the Kinetograph"},
        {"tt0000008", "Edison Kinetoscopic Record of a Sneeze"},
        {"tt0000009", "Miss Jerry"},
        {"tt0000010", "Leaving the Factory"}};

    BTree<FixedMovieIdString> movieIdIndex("movieId_index2000.bin", &bm);
    for (const auto &entry : testMovieId) {
        string searchId = entry.first;
        string searchTitle = entry.second;

        FixedMovieIdString searchIdStr(searchId);
        auto idResults = movieIdIndex.search(searchIdStr);

        cout << "Found " << idResults.size() << " results for movieId " << searchId << "\n";
        for (const auto &rid : idResults) {
            int pageId = rid.first;
            int slotId = rid.second;
            cout << "Found movie at page " << pageId << ", slot " << slotId << "\n";
            Page *page = bm.getPage(pageId, movieTableInfo.filePath);
            Row *row = page->getRow(slotId);
            string foundId(reinterpret_cast<const char *>(row->movieId.data()),
                           row->movieId.size());
            string foundTitle(reinterpret_cast<const char *>(row->title.data()), row->title.size());
            assert(foundId == searchId);
            FixedTitleSizeString foundTitleStr(foundTitle);
            FixedTitleSizeString searchTitleStr(searchTitle);
            assert(foundTitleStr == searchTitleStr);
            delete row;
            bm.unpinPage(pageId, movieTableInfo.filePath);
        }
    }
}

// Test C4: Range search test
void test_C4_range_search(DatabaseCatalog &catalog)
{
    cout << "\n=== Test C4: Range search test ===\n";
    BufferManager bm(20);
    auto movieTableInfo = catalog.getTable("Movie");
    bm.registerFile(movieTableInfo.filePath);

    // Test title index range search
    cout << "Testing title index range search...\n";
    BTree<FixedTitleSizeString> titleIndex("title_index2000.bin", &bm);
    string startTitle = "The";
    string endTitle = "Thf";
    FixedTitleSizeString startTitleStr(startTitle);
    FixedTitleSizeString endTitleStr(endTitle);
    auto titleResults = titleIndex.rangeSearch(startTitleStr, endTitleStr);

    cout << "Found " << titleResults.size() << " results in title range [" << startTitle << ", "
         << endTitle << "]\n";
    for (const auto &rid : titleResults) {
        Page *page = bm.getPage(rid.first, "movie2000.bin");
        Row *row = page->getRow(rid.second);
        string foundTitle(reinterpret_cast<const char *>(row->title.data()), row->title.size());
        cout << "Found movie: " << foundTitle << "\n";
        assert(foundTitle >= startTitle && foundTitle <= endTitle);
        delete row;
        bm.unpinPage(rid.first, "movie2000.bin");
    }

    // Test movieId index range search
    cout << "\nTesting movieId index range search...\n";
    BTree<FixedMovieIdString> movieIdIndex("movieId_index2000.bin", &bm);
    string startId = "tt0000010";
    string endId = "tt0000100";
    FixedMovieIdString startIdStr(startId);
    FixedMovieIdString endIdStr(endId);
    auto idResults = movieIdIndex.rangeSearch(startIdStr, endIdStr);

    cout << "Found " << idResults.size() << " results in movieId range [" << startId << ", "
         << endId << "]\n";
    for (const auto &rid : idResults) {
        Page *page = bm.getPage(rid.first, "movie2000.bin");
        Row *row = page->getRow(rid.second);
        string movieId(reinterpret_cast<const char *>(row->movieId.data()), row->movieId.size());
        FixedMovieIdString foundIdStr(movieId);

        assert(foundIdStr >= startIdStr && foundIdStr <= endIdStr);
        delete row;
        bm.unpinPage(rid.first, "movie2000.bin");
    }
}

// Test P1: Performance comparison with title index
void test_P1_performance()
{
    cout << "\n=== Test P1: Title index performance test ===\n";
    BufferManager bm(20);
    BTree<string> titleIndex("title_index2000.bin", &bm);

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
        Page *page = bm.getPage(pageId, "movie2000.bin");
        if (!page) break;

        for (int slotId = 0; slotId < page->getNumRecords(); slotId++) {
            Row *row = page->getRow(slotId);
            if (row) {
                string title(reinterpret_cast<const char *>(row->title.data()), row->title.size());
                if (title == searchTitle) {
                    scanResults.push_back({pageId, slotId});
                }
                delete row;
            }
        }
        bm.unpinPage(pageId, "movie2000.bin");
        pageId++;
    }

    auto scanEnd = chrono::high_resolution_clock::now();
    auto scanTime = chrono::duration_cast<chrono::microseconds>(scanEnd - scanStart).count();

    cout << "Table scan time: " << scanTime << "µs\n";
    cout << "Found " << scanResults.size() << " results using table scan\n";
    cout << "Speed-up: " << (double)scanTime / indexTime << "x\n";
}

// Test P2: Performance comparison with movieId index
void test_P2_performance()
{
    cout << "\n=== Test P2: MovieId index performance test ===\n";
    BufferManager bm(20);
    BTree<int> movieIdIndex("movieId_index2000.bin", &bm);

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
        Page *page = bm.getPage(pageId, "movie2000.bin");
        if (!page) break;

        for (int slotId = 0; slotId < page->getNumRecords(); slotId++) {
            Row *row = page->getRow(slotId);
            if (row) {
                try {
                    string movieIdStr(reinterpret_cast<const char *>(row->movieId.data()),
                                      row->movieId.size());
                    int movieId = stoi(movieIdStr);
                    if (movieId == searchId) {
                        scanResults.push_back({pageId, slotId});
                    }
                } catch (const exception &e) {
                    // Skip invalid movieIds
                }
                delete row;
            }
        }
        bm.unpinPage(pageId, "movie2000.bin");
        pageId++;
    }

    auto scanEnd = chrono::high_resolution_clock::now();
    auto scanTime = chrono::duration_cast<chrono::microseconds>(scanEnd - scanStart).count();

    cout << "Table scan time: " << scanTime << "µs\n";
    cout << "Found " << scanResults.size() << " results using table scan\n";
    cout << "Speed-up: " << (double)scanTime / indexTime << "x\n";
}

int main()
{
    try {
        // First check if movie2000.bin exists
        ifstream f("movie2000.bin", ios::binary);
        if (!f.good()) {
            cout << "movie2000.bin not found. Loading movie data...\n";
            loadMovieData();
        }
        f.close();

        // Create the catalog and add Movies table metadata.
        DatabaseCatalog catalog;
        DatabaseCatalog::TableInfo movieTableInfo = {"Movie", "movie2000.bin"};
        catalog.addTable(movieTableInfo);

        // Correctness Tests
        test_C1_title_index(catalog);
        test_C2_movieId_index(catalog);
        test_C3_point_search(catalog);
        test_C4_range_search(catalog);

        // // Performance Tests
        // test_P1_performance();
        // test_P2_performance();

        cout << "\nAll tests completed successfully!\n";
    } catch (const exception &e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    return 0;
}