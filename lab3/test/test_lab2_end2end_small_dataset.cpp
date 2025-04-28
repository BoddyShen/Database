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
    Page<MovieRow> *appendPage = bm.createPage<MovieRow>("movie2000.bin");
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
            appendPage = bm.createPage<MovieRow>("movie2000.bin");
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
// FixedStringType is either FixedTitleSizeString or FixedMovieIdString, since the BTrees only
// accept fixed-size keys.
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
        Page<MovieRow> *page = bm.getPage<MovieRow>(pageId, movieFile);
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

    string titleIndexName = catalog.getIndex("title_index2000.bin").filePath;
    BTree<FixedTitleSizeString> titleIndex(titleIndexName, &bm);

    for (const string &searchTitle : testTitles) {
        FixedTitleSizeString searchTitleStr(searchTitle);

        cout << "Searching for movie with title: " << searchTitle << endl;
        auto titleResults = titleIndex.search(searchTitleStr);
        cout << "Found " << titleResults.size() << " results for title '" << searchTitle << "'\n";
        for (const auto &rid : titleResults) {
            int pageId = rid.first;
            int slotId = rid.second;

            Page<MovieRow> *page = bm.getPage<MovieRow>(pageId, movieTableInfo.filePath);
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
            Page<MovieRow> *page = bm.getPage<MovieRow>(pageId, movieTableInfo.filePath);
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

    string titleIndexFilePath = catalog.getIndex("title_index2000.bin").filePath;
    string movieIdIndexFilePath = catalog.getIndex("movieId_index2000.bin").filePath;

    // Test title index range search
    cout << "Testing title index range search...\n";
    BTree<FixedTitleSizeString> titleIndex(titleIndexFilePath, &bm);
    string startTitle = "The";
    string endTitle = "Thf";
    FixedTitleSizeString startTitleStr(startTitle);
    FixedTitleSizeString endTitleStr(endTitle);
    auto titleResults = titleIndex.rangeSearch(startTitleStr, endTitleStr);

    cout << "Found " << titleResults.size() << " results in title range [" << startTitle << ", "
         << endTitle << "]\n";
    for (const auto &rid : titleResults) {
        Page<MovieRow> *page = bm.getPage<MovieRow>(rid.first, movieTableInfo.filePath);
        Row *row = page->getRow(rid.second);
        string foundTitle(reinterpret_cast<const char *>(row->title.data()), row->title.size());
        cout << "Found movie: " << foundTitle << "\n";
        FixedTitleSizeString foundTitleStr(foundTitle);
        assert(foundTitleStr >= startTitleStr && foundTitleStr <= endTitleStr);
        delete row;
        bm.unpinPage(rid.first, movieTableInfo.filePath);
    }

    // Test movieId index range search
    cout << "\nTesting movieId index range search...\n";
    BTree<FixedMovieIdString> movieIdIndex(movieIdIndexFilePath, &bm);
    string startId = "tt0000010";
    string endId = "tt0000100";
    FixedMovieIdString startIdStr(startId);
    FixedMovieIdString endIdStr(endId);
    auto idResults = movieIdIndex.rangeSearch(startIdStr, endIdStr);

    cout << "Found " << idResults.size() << " results in movieId range [" << startId << ", "
         << endId << "]\n";
    for (const auto &rid : idResults) {
        Page<MovieRow> *page = bm.getPage<MovieRow>(rid.first, movieTableInfo.filePath);
        Row *row = page->getRow(rid.second);
        string movieId(reinterpret_cast<const char *>(row->movieId.data()), row->movieId.size());
        FixedMovieIdString foundIdStr(movieId);

        assert(foundIdStr >= startIdStr && foundIdStr <= endIdStr);
        delete row;
        bm.unpinPage(rid.first, movieTableInfo.filePath);
    }
}

// Test P1: Perform a range query using two methods: (1) a direct scan of the Movies table, and
// (2) get Rids from the title index, then access the rows from the Movies table and return
// them.

// The results are printed to the console and saved to a CSV file for plotting.
void exportResultsToCSV(const vector<double> &selectivities, const vector<double> &directTimes,
                        const vector<double> &indexTimes, const vector<double> &ratios,
                        const string &filename)
{
    ofstream out(filename);
    out << "Selectivity,DirectTime,IndexTime,Ratio\n";
    for (size_t i = 0; i < selectivities.size(); i++) {
        out << selectivities[i] << "," << directTimes[i] << "," << indexTimes[i] << "," << ratios[i]
            << "\n";
    }
    out.close();
}

// Helper function for method 1: direct scan of Movies table.
tuple<double, int, int> directScanRangeQuery(BufferManager &bm, const string &movieFile,
                                             const string &startTitle, const string &endTitle)
{
    auto startTime = std::chrono::high_resolution_clock::now();
    int pageId = 0;
    int matchingRows = 0;
    int totalRows = 0;

    ifstream f(movieFile, ios::binary);
    f.seekg(0, ios::end);
    std::streampos fileSize = f.tellg();
    f.close();

    FixedTitleSizeString startTitleStr(startTitle);
    FixedTitleSizeString endTitleStr(endTitle);

    while (pageId * MAX_PAGE_SIZE < fileSize) {
        Page<MovieRow> *page = bm.getPage<MovieRow>(pageId, movieFile);
        if (!page) break;
        for (int slotId = 0; slotId < page->getNumRecords(); slotId++) {
            Row *row = page->getRow(slotId);
            if (row) {
                totalRows++;
                string title(reinterpret_cast<const char *>(row->title.data()), row->title.size());
                FixedTitleSizeString titleStr(title);
                if (titleStr >= startTitleStr && titleStr <= endTitleStr) {
                    matchingRows++;
                }
                // if (title >= startTitle && title <= endTitle) {
                //     matchingRows++;
                // }
                delete row;
            }
        }
        bm.unpinPage(pageId, movieFile);
        pageId++;
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    double durationMs =
        std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    cout << "[Direct Scan] Range [" << startTitle << ", " << endTitle << "] "
         << "matched " << matchingRows << " rows in " << durationMs << " ms.\n";
    return {durationMs, matchingRows, totalRows};
}

// Helper function for method 2: use the title index.
pair<double, int> indexBasedRangeQuery(BufferManager &bm, const string &movieFile,
                                       const string &startTitle, const string &endTitle,
                                       const string &indexFilePath)
{
    auto startTime = std::chrono::high_resolution_clock::now();

    BTree<FixedTitleSizeString> titleIndex(indexFilePath, &bm);
    FixedTitleSizeString startTitleStr(startTitle);
    FixedTitleSizeString endTitleStr(endTitle);
    vector<Rid> rids = titleIndex.rangeSearch(startTitleStr, endTitleStr);
    cout << "Found " << rids.size() << " results in title range [" << startTitle << ", " << endTitle
         << "]\n";

    int matchingRows = 0;
    for (const auto &rid : rids) {
        int pageId = rid.first;
        int slotId = rid.second;
        Page<MovieRow> *page = bm.getPage<MovieRow>(pageId, movieFile);
        if (!page) continue;
        Row *row = page->getRow(slotId);
        if (row) {
            string title(reinterpret_cast<const char *>(row->title.data()), row->title.size());
            FixedTitleSizeString titleStr(title);
            if (titleStr >= startTitleStr && titleStr <= endTitleStr) {
                matchingRows++;
            }
            delete row;
        }
        bm.unpinPage(pageId, movieFile);
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    double durationMs =
        std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    cout << "[Index Based] Range [" << startTitle << ", " << endTitle << "] "
         << "matched " << matchingRows << " rows in " << durationMs << " ms.\n";
    return {durationMs, matchingRows};
}

void test_P1(DatabaseCatalog &catalog)
{
    cout << "\n=== Test P1: Title index performance test ===\n";
    BufferManager bm(20);
    auto movieTableInfo = catalog.getTable("Movie");
    bm.registerFile(movieTableInfo.filePath);
    string indexFilePath = catalog.getIndex("title_index2000.bin").filePath;

    // Test title range search with increasing ranges
    // TODO: Handle different matched row counts
    vector<pair<string, string>> ranges = {
        {"A", "B"}, {"A", "C"}, {"A", "G"}, {"A", "M"}, {"A", "Z"}};

    // Vectors to store results for plotting.
    vector<double> selectivities; // fraction of rows
    vector<double> timeDirect;
    vector<double> timeIndex;
    vector<double> timeRatio; // direct / index
    int totalRows = 0;

    for (auto &range : ranges) {
        // Run the two query methods.
        // return <time, matchedRow. totalRows>
        tuple<double, int, int> data1 =
            directScanRangeQuery(bm, movieTableInfo.filePath, range.first, range.second);
        pair<double, int> data2 = indexBasedRangeQuery(bm, movieTableInfo.filePath, range.first,
                                                       range.second, indexFilePath);
        double t1 = std::get<0>(data1);
        double t2 = data2.first;
        double ratio = (t2 != 0) ? (t1 / t2) : 0;
        int matchedRow = std::get<1>(data1);
        totalRows = std::get<2>(data1);
        // Save the measurements.
        selectivities.push_back(matchedRow / static_cast<double>(totalRows));
        timeDirect.push_back(t1);
        timeIndex.push_back(t2);
        timeRatio.push_back(ratio);
    }

    // Output the collected data (you can then use Python/matplotlib or Excel to plot)
    cout << "Selectivity, DirectTime(ms), IndexTime(ms), Ratio\n";
    for (size_t i = 0; i < selectivities.size(); i++) {
        cout << selectivities[i] << ", " << timeDirect[i] << ", " << timeIndex[i] << ", "
             << timeRatio[i] << "\n";
    }

    // Export to CSV for plotting
    exportResultsToCSV(selectivities, timeDirect, timeIndex, timeRatio, "test_P1_result_2000.csv");
}

// Test P2: Performance comparison with movieId index
// Helper function for method 1: Direct scan of the Movies table based on movieId.
tuple<double, int, int> directScanRangeQuery_movieId(BufferManager &bm, const string &movieFile,
                                                     const FixedMovieIdString &startId,
                                                     const FixedMovieIdString &endId)
{
    auto startTime = std::chrono::high_resolution_clock::now();
    int pageId = 0;
    int matchingRows = 0;
    int totalRows = 0;

    // Get file size
    ifstream f(movieFile, ios::binary);
    f.seekg(0, ios::end);
    std::streampos fileSize = f.tellg();
    f.close();

    while (pageId * MAX_PAGE_SIZE < fileSize) {
        Page<MovieRow> *page = bm.getPage<MovieRow>(pageId, movieFile);
        if (!page) break;
        for (int slotId = 0; slotId < page->getNumRecords(); slotId++) {
            Row *row = page->getRow(slotId);
            if (row) {
                totalRows++;
                // Construct a fixed-size key from the row's movieId field.
                FixedMovieIdString key(string(reinterpret_cast<const char *>(row->movieId.data()),
                                              row->movieId.size()));
                if (key >= startId && key <= endId) {
                    matchingRows++;
                }
                delete row;
            }
        }
        bm.unpinPage(pageId, movieFile);
        pageId++;
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    double durationMs =
        std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    cout << "[Direct Scan] Range [" << startId.toString() << ", " << endId.toString()
         << "] matched " << matchingRows << " rows in " << durationMs << " ms.\n";
    return {durationMs, matchingRows, totalRows};
}

// Helper function for method 2: Use the movieId index.
pair<double, int> indexBasedRangeQuery_movieId(BufferManager &bm, const string &movieFile,
                                               const FixedMovieIdString &startId,
                                               const FixedMovieIdString &endId)
{
    auto startTime = std::chrono::high_resolution_clock::now();

    // Assume the movieId index has been built and is stored in "movieId_index.bin"
    // and that its key type is FixedTitleSizeString.
    BTree<FixedMovieIdString> movieIdIndex("movieId_index2000.bin", &bm);

    // Get all record IDs (RIDs) in the range.
    vector<Rid> rids = movieIdIndex.rangeSearch(startId, endId);

    int matchingRows = 0;
    for (const auto &rid : rids) {
        int pageId = rid.first;
        int slotId = rid.second;
        Page<MovieRow> *page = bm.getPage<MovieRow>(pageId, movieFile);
        if (!page) continue;
        Row *row = page->getRow(slotId);
        if (row) {
            FixedMovieIdString key(
                string(reinterpret_cast<const char *>(row->movieId.data()), row->movieId.size()));
            if (key >= startId && key <= endId) {
                matchingRows++;
            }
            delete row;
        }
        bm.unpinPage(pageId, movieFile);
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    double durationMs =
        std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    cout << "[Index Based] Range [" << startId.toString() << ", " << endId.toString()
         << "] matched " << matchingRows << " rows in " << durationMs << " ms.\n";
    return {durationMs, matchingRows};
}

// Test P2: Compare direct scan versus index-based range query on movieId.
void test_P2(DatabaseCatalog &catalog)
{
    cout << "\n=== Test P2: Range query test on movieId attribute ===\n";
    auto movieTableInfo = catalog.getTable("Movie");
    string movieFile = movieTableInfo.filePath;

    BufferManager bm(20);
    bm.registerFile(movieFile);

    // Define a set of range queries with increasing selectivity.
    // For example, these ranges are defined by fixed movieId values.
    vector<pair<FixedMovieIdString, FixedMovieIdString>> ranges = {
        {FixedMovieIdString("tt0000001"), FixedMovieIdString("tt0000100")},
        {FixedMovieIdString("tt0000001"), FixedMovieIdString("tt0000500")},
        {FixedMovieIdString("tt0000001"), FixedMovieIdString("tt0001000")},
        {FixedMovieIdString("tt0000001"), FixedMovieIdString("tt0001500")}};

    // Vectors for plotting data.
    vector<double> selectivities; // e.g., fraction of rows matched (if you have total row count)
    vector<double> directTimes;
    vector<double> indexTimes;
    vector<double> ratioTimes;

    // For each range, run both query methods.
    for (auto &range : ranges) {
        // {time, matchedRows, totalRows}
        tuple<double, int, int> directResult =
            directScanRangeQuery_movieId(bm, movieFile, range.first, range.second);
        pair<double, int> indexResult =
            indexBasedRangeQuery_movieId(bm, movieFile, range.first, range.second);
        double tDirect = std::get<0>(directResult);
        double tIndex = indexResult.first;

        // Compute selectivity as the fraction of rows matched in a direct scan.
        int matchedRows = std::get<1>(directResult);
        int totalRows = std::get<2>(directResult);
        double selectivity = static_cast<double>(matchedRows) / static_cast<double>(totalRows);
        double ratio = (tIndex != 0) ? (tDirect / tIndex) : 0;

        selectivities.push_back(selectivity);
        directTimes.push_back(tDirect);
        indexTimes.push_back(tIndex);
        ratioTimes.push_back(ratio);

        cout << "Range [" << range.first.toString() << ", " << range.second.toString() << "]: "
             << "selectivity=" << selectivity << ", direct=" << tDirect << "ms, index=" << tIndex
             << "ms, ratio=" << ratio << "\n";
    }

    // Print out data for plotting.
    cout << "\nSelectivity, DirectTime(ms), IndexTime(ms), Ratio\n";
    for (size_t i = 0; i < selectivities.size(); i++) {
        cout << selectivities[i] << ", " << directTimes[i] << ", " << indexTimes[i] << ", "
             << ratioTimes[i] << "\n";
    }

    // Export results to CSV for further analysis.
    exportResultsToCSV(selectivities, directTimes, indexTimes, ratioTimes,
                       "test_P2_result_2000.csv");
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

        // Performance Tests
        test_P1(catalog);
        test_P2(catalog);

        cout << "\nAll tests completed successfully!\n";
    } catch (const exception &e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    return 0;
}