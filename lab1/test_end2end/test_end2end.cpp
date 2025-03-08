#include "BufferManager.h"
#include "Constants.h"
#include "Page.h"
#include "Row.h"
#include "Utilities.h"
#include <cassert>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

void loading()
{
    remove(DB_FILE.c_str());
    BufferManager bm(FRAME_SIZE, DB_FILE);

    // Open the TSV file
    ifstream tsvFile("title.basics.tsv");
    if (!tsvFile.is_open()) {
        cerr << "Failed to open title.basics.tsv" << endl;
        return;
    }

    string header;
    getline(tsvFile, header);

    // Create the first append page, all data will be inserted into this page
    Page *appendPage = bm.createPage();
    int appendPid = appendPage->getPid();
    cout << "Initial append page id: " << appendPid << endl;

    // 逐行讀取 TSV 檔案
    string line;
    int loadedRows = 0;
    while (getline(tsvFile, line)) {
        // 使用 tab 分隔解析行資料
        loadedRows++;
        istringstream iss(line);
        vector<string> tokens;
        string token;
        while (getline(iss, token, '\t')) {
            tokens.push_back(token);
        }
        // 假設 tconst 在第0欄，primaryTitle 在第2欄（請根據實際檔案格式調整）
        if (tokens.size() < 3) continue;
        string movieId = tokens[0];
        string title = tokens[2];

        // 截斷資料到固定長度：movieId 9 個字元，title 30 個字元
        if (movieId.size() > 9) movieId = movieId.substr(0, 9);
        if (title.size() > 30) title = title.substr(0, 30);

        // 建立 Row 物件，假設 Row 有接受 C-string 的建構子
        Row row(movieId.c_str(), title.c_str());

        // 若當前頁面已滿，先 unpin，再建立新頁面
        if (appendPage->isFull()) {
            bm.unpinPage(appendPid);
            appendPage = bm.createPage();
            appendPid = appendPage->getPid();
            cout << "Loaded " << loadedRows << " rows" << endl;
            cout << "Created new append page, id: " << appendPid << endl;
        }

        // 插入資料到當前附加頁面
        int rowId = appendPage->insertRow(row);
        if (rowId == -1) {
            cerr << "Failed to insert row into page " << appendPid << endl;
        }
    }

    tsvFile.close();

    // // 載入結束後，解除最後一個附加頁面的 pin
    bm.unpinPage(appendPid);

    cout << "Loaded " << loadedRows << " rows into the Movies table." << endl;
}

void querying()
{
    // This test case is similar to the one in test_buffer_manager.cpp
    cout << "Test: Interleaved Insert and Query" << endl;
    remove(QUERY_DB_FILE.c_str());
    // Create a BufferManager with 2 frames and a test database file.
    BufferManager bm(2, QUERY_DB_FILE);

    cout << "1. Create a new page and insert 2 rows." << endl;
    Page *p = bm.createPage();
    int pid0 = p->getPid();
    cout << "Created page with pageId " << pid0 << endl;

    // Insert a few rows into the page (but do not fill it completely
    bm.markDirty(pid0);
    int rowId1 = p->insertRow(Row("tt0000001", "Carmencita"));
    int rowId2 = p->insertRow(Row("tt0000002", "Le clown et ses chiens"));
    assert(rowId1 == 0);
    assert(rowId2 == 1);
    cout << "Inserted rows " << rowId1 << " and " << rowId2 << " into page " << pid0 << endl;

    // Check that the rows were inserted correctly.
    Page *p0 = bm.getPage(pid0);
    assert(p0 == p);
    Row *p0r0 = p0->getRow(0);
    string p0r0Str = Utilities::rowToString(*p0r0);
    assert(p0r0Str == "Movie ID: tt0000001, Title: Carmencita");
    Row *p0r1 = p0->getRow(1);
    string p0r1Str = Utilities::rowToString(*p0r1);
    assert(p0r1Str == "Movie ID: tt0000002, Title: Le clown et ses chiens");

    // Unpin the page so it can be evicted.
    bm.unpinPage(pid0);
    bm.unpinPage(pid0);
    cout << "Check that the rows were inserted correctly in page: " << pid0 << endl;
    cout << endl;

    cout << "2. Create second page." << endl;
    // Create second page.
    Page *p1 = bm.createPage();
    int pid1 = p1->getPid();
    cout << "Created page with pageId " << pid1 << endl;
    // Insert a row into p1.
    bm.markDirty(pid1);
    int rowId_p1 = p1->insertRow(Row("tt0000003", "Poor Pierrot"));
    assert(rowId_p1 == 0);
    bm.unpinPage(pid1);
    cout << endl;

    cout << "3. Create one more page, which will force eviction since capacity is 2." << endl;
    Page *p2 = bm.createPage();
    int pid2 = p2->getPid();
    cout << "Created page with pageId " << pid2 << " (trigger eviction)" << endl;
    // Insert a row into p2.
    bm.markDirty(pid2);
    int rowId_p2 = p2->insertRow(Row("tt0000004", "Un bon bock"));
    assert(rowId_p2 == 0);
    bm.unpinPage(pid2);
    cout << endl;

    cout << "4. Access the original page, pid=0." << endl;
    Page *p0_reloaded = bm.getPage(pid0);
    cout << "Number of records in page id " << pid0 << ": " << p0_reloaded->getNumRecords() << endl;
    // The page was evicted, p_reloaded will be a new pointer (not equal to p).
    assert(p0_reloaded != p0);
    bm.unpinPage(pid0);
    cout << endl;

    cout << "5. Insert an additional row into the reloaded page." << endl;
    bm.markDirty(pid0);
    int rowId3 = p0_reloaded->insertRow(Row("tt0000005", "Blacksmith Scene"));
    cout << "Inserted row " << rowId3 << " into page " << pid0 << " after reloading." << endl;
    bm.unpinPage(pid0);
    cout << endl;

    cout << "6. Validate the content of the page." << endl;
    cout << "Page id" << pid0 << " contents :" << endl;
    for (int i = 0; i < p0_reloaded->getNumRecords(); i++) {
        Row *r = p0_reloaded->getRow(i);
        cout << Utilities::rowToString(*r) << endl;
        delete r;
    }

    Row *p0r0_reloaded = p0_reloaded->getRow(rowId1);
    string p0r0Str_reloaded = Utilities::rowToString(*p0r0_reloaded);
    string expectedR0 = "Movie ID: tt0000001, Title: Carmencita";
    assert(p0r0Str_reloaded == expectedR0);
    cout << "Assert Row " << rowId1 << " reloaded successfully." << endl;

    // Retrieve row 1 (should be "Movie Two").
    Row *p0r1_reloaded = p0_reloaded->getRow(rowId2);
    string p0r1Str_reloaded = Utilities::rowToString(*p0r1_reloaded);
    string expectedR1 = "Movie ID: tt0000002, Title: Le clown et ses chiens";
    assert(p0r1Str_reloaded == expectedR1);
    cout << "Assert Row " << rowId2 << " reloaded successfully." << endl;

    // Retrieve the additional inserted row (should be "Movie Five").
    Row *p0r2_reloaded = p0_reloaded->getRow(rowId3);
    string p0r2Str_reloaded = Utilities::rowToString(*p0r2_reloaded);
    string expectedR2 = "Movie ID: tt0000005, Title: Blacksmith Scene";
    assert(p0r2Str_reloaded == expectedR2);
    cout << "Assert Row " << rowId3 << " reloaded successfully." << endl;

    cout << "Test interleaved insert and query passed." << endl;
}

int main()
{
    loading();
    querying();
    cout << "End-to-end test complete." << endl;
    return 0;
}