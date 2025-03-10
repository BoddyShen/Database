#include "BufferManager.h"
#include "Constants.h"
#include "Page.h"
#include "Row.h"
#include "Utilities.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

int loading()
{
    // Remove existing database file to start with a clean slate.
    remove(DB_FILE.c_str());
    BufferManager bm(FRAME_SIZE, DB_FILE);

    // Open the TSV file
    ifstream tsvFile("title.basics.tsv");
    if (!tsvFile.is_open()) {
        cerr << "Failed to open title.basics.tsv" << endl;
        return 0;
    }

    string header;
    getline(tsvFile, header);
    cout << "Header: " << header << endl;

    // Create the first append page; all data will be inserted into this page.
    Page *appendPage = bm.createPage();
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
            bm.unpinPage(appendPid);
            appendPage = bm.createPage();
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
    bm.unpinPage(appendPid);

    cout << "Loaded " << loadedRows << " rows into the Movies table." << endl;
    return appendPid;
}

void querying(int pids)
{
    // This test case is similar to the one in test_buffer_manager.cpp but using real movie data.
    if (pids == 0) {
        cerr << "No pages were loaded, skipping querying test." << endl;
        return;
    }
    srand(static_cast<unsigned>(time(0)));

    cout << "Test: Interleaved Insert and Query" << endl;
    // Create a BufferManager with 2 frames and a test database file.
    BufferManager bm(2, DB_FILE);
    Page *p = bm.createPage();
    int append_pid = p->getPid();
    bm.unpinPage(append_pid);

    cout << "Created page with pageId " << append_pid << endl;
    cout << p->getNumRecords() << " records in page." << endl;

    // Insert a few rows into the page (but do not fill it completely).
    int loop_times = 0;
    while (loop_times < 3) {
        loop_times++;
        // Insert a row into the page (but do not fill it completely).
        bm.markDirty(append_pid);
        Page *p_current = bm.getPage(append_pid);
        string id = "id00000" + to_string(loop_times);
        string title = "Test Movie Title " + to_string(loop_times);
        int rowId = p_current->insertRow(Row(id.c_str(), title.c_str()));
        cout << "Inserted row " << rowId << ": " << id << ", " << title << std::endl;
        bm.unpinPage(append_pid);

        // Check that the row was inserted correctly.
        Page *p0 = bm.getPage(append_pid);
        string rowStr = Utilities::rowToString(*p0->getRow(rowId));
        assert(rowStr == "Movie ID: " + id + ", Title: " + title);
        bm.unpinPage(append_pid);

        // Query random pages to evict the append page.
        for (int i = 0; i < 5; i++) {
            int randPid = rand() % pids;
            cout << "Querying page " << randPid << "..." << endl;
            (void)bm.getPage(randPid);
            bm.unpinPage(randPid);
        }

        // Check that the row was inserted correctly after eviction.
        p0 = bm.getPage(append_pid);
        rowStr = Utilities::rowToString(*p0->getRow(rowId));
        assert(rowStr == "Movie ID: " + id + ", Title: " + title);
        bm.unpinPage(append_pid);

        cout << loop_times << " rows inserted." << endl;
        cout << p->getNumRecords() << " records in page." << endl;
    }
    cout << "Test interleaved insert and query passed." << endl;
}

int main()
{
    int pids = loading();
    querying(pids);
    cout << "End-to-end test complete." << endl;
    return 0;
}