#include "BufferManager.h"
#include "Constants.h"
#include "Page.h"
#include "Row.h"
#include "Utilities.h"
#include <cassert>
#include <cstdio>
#include <iostream>

using namespace std;

void testCreateAndGetPage()
{
    cout << "Test: createPage and getPage" << endl;

    // Create a small BufferManager with 2 frames.
    BufferManager bm(2, TEST_DB_FILE);

    // 1. Create the first page.
    Page *p1 = bm.createPage();
    assert(p1 != nullptr);
    // bm.printStatus();
    cout << "Created page p1." << endl;

    // After using p1, unpin it.
    bm.unpinPage(0);

    // 2. Get the page using getPage (p1 should be in the buffer pool).
    Page *p1_again = bm.getPage(0);
    assert(p1_again != nullptr);
    // It should return the same page pointer.
    assert(p1 == p1_again);
    cout << "getPage(0) returned the same page as p1." << endl;
    // Unpin p1 again after access.
    bm.unpinPage(0);

    // 3. Create the second page.
    Page *p2 = bm.createPage();
    assert(p2 != nullptr);
    cout << "Created page p2." << endl;
    // bm.printStatus();
    // Unpin page 1 (assuming p2 gets assigned page id 1)
    bm.unpinPage(1);

    // 4. Create the third page.
    // Since the buffer pool has only 2 frames, this should trigger eviction.
    Page *p3 = bm.createPage();
    assert(p3 != nullptr);
    cout << "Created page p3, triggering eviction." << endl;
    // Unpin page 2 (p3's page id) after creation.
    bm.unpinPage(2);

    // // To simulate eviction, we need to ensure that a page's pin count is 0.
    // // Unpin page 0 again if needed.
    bm.unpinPage(0);

    // Now, attempt to get page 0.
    Page *p1_loaded = bm.getPage(0);
    // p1 should be evicted, p1_loaded should not be the same pointer as p1.
    assert(p1_loaded != p1);
    bm.unpinPage(0);

    // 5. Test markDirty and unpinPage functionality.
    bm.markDirty(1); // Mark page 1 as dirty.
    bm.unpinPage(1); // Unpin page 1.

    cout << "Test createPage and getPage passed!" << endl;
}

void testInterleavedInsertAndQuery()
{
    cout << "Test: Interleaved Insert and Query" << endl;

    // Create a BufferManager with 2 frames and a test database file.
    BufferManager bm(2, TEST_DB_FILE);

    // 1. Create a new page.
    Page *p = bm.createPage();
    int pid0 = p->getPid();
    cout << "Created page with pageId " << pid0 << endl;

    // 2. Insert a few rows into the page (but do not fill it completely).
    bm.markDirty(pid0);
    int rowId1 = p->insertRow(Row("id000001", "Movie One"));
    int rowId2 = p->insertRow(Row("id000002", "Movie Two"));
    assert(rowId1 == 0);
    assert(rowId2 == 1);
    cout << "Inserted rows " << rowId1 << " and " << rowId2 << " into page " << pid0 << endl;

    // Check that the rows were inserted correctly.
    Page *p0 = bm.getPage(pid0);
    assert(p0 == p);
    Row *p0r0 = p0->getRow(0);
    string p0r0Str = Utilities::rowToString(*p0r0);
    assert(p0r0Str == "Movie ID: id000001, Title: Movie One");
    Row *p0r1 = p0->getRow(1);
    string p0r1Str = Utilities::rowToString(*p0r1);
    assert(p0r1Str == "Movie ID: id000002, Title: Movie Two");

    // Unpin the page so it can be evicted.
    bm.unpinPage(pid0);
    bm.unpinPage(pid0);

    // 3. Create additional pages to fill up the buffer.
    // Create second page.
    Page *p1 = bm.createPage();
    int pid1 = p1->getPid();
    cout << "Created page with pageId " << pid1 << endl;
    // Insert a row into p1.
    bm.markDirty(pid1);
    int rowId_p1 = p1->insertRow(Row("id000003", "Movie Three"));
    assert(rowId_p1 == 0);
    bm.unpinPage(pid1);

    // Create one more page, which will force eviction since capacity is 2.
    Page *p2 = bm.createPage();
    int pid2 = p2->getPid();
    cout << "Created page with pageId " << pid2 << " (trigger eviction)" << endl;
    // Insert a row into p2.
    bm.markDirty(pid2);
    int rowId_p2 = p2->insertRow(Row("id000004", "Movie Four"));
    assert(rowId_p2 == 0);
    bm.unpinPage(pid2);

    // 4. Access the original page.
    Page *p0_reloaded = bm.getPage(pid0);
    cout << "Number of records: " << p0_reloaded->getNumRecords() << endl;
    // The page was evicted, p_reloaded will be a new pointer (not equal to p).
    assert(p0_reloaded != p0);
    bm.unpinPage(pid0);

    // 5. Insert an additional row into the reloaded page.
    bm.markDirty(pid0);
    int rowId3 = p0_reloaded->insertRow(Row("id000005", "Movie Five"));
    cout << "Inserted row " << rowId3 << " into page " << pid0 << " after reloading." << endl;
    bm.unpinPage(pid0);

    // 6. Validate the content of the page.
    // Retrieve row 0 (should be "Movie One").
    Row *p0r0_reloaded = p0_reloaded->getRow(0);
    string p0r0Str_reloaded = Utilities::rowToString(*p0r0_reloaded);
    cout << p0r0Str_reloaded << endl;
    string expectedR0 = "Movie ID: id000001, Title: Movie One";
    assert(p0r0Str_reloaded == expectedR0);
    cout << "Assert Row 0 reloaded successfully." << endl;

    // Retrieve row 1 (should be "Movie Two").
    Row *p0r1_reloaded = p0_reloaded->getRow(1);
    string p0r1Str_reloaded = Utilities::rowToString(*p0r1_reloaded);
    string expectedR1 = "Movie ID: id000002, Title: Movie Two";
    assert(p0r1Str_reloaded == expectedR1);

    // Retrieve the additional inserted row (should be "Movie Five").
    Row *p0r2_reloaded = p0_reloaded->getRow(rowId3);
    string p0r2Str_reloaded = Utilities::rowToString(*p0r2_reloaded);
    string expectedR2 = "Movie ID: id000005, Title: Movie Five";
    assert(p0r2Str_reloaded == expectedR2);

    cout << "Test interleaved insert and query passed." << endl;
}

void testNoEmptyFrame()
{
    cout << "Test: No Empty Frame when all pages are pinned" << endl;

    // Create a BufferManager with only 1 frame and a test database file.
    BufferManager bm(1, TEST_DB_FILE);

    // 1. Create a page. This page will be pinned (its pin count will be > 0).
    Page *p = bm.createPage();
    assert(p != nullptr);
    int pid = p->getPid();
    cout << "Created page with pageId " << pid << endl;

    // Do not unpin the page so that it remains pinned.
    // 2. Attempt to create another page.
    // Since the buffer pool only has 1 frame and that frame is already pinned,
    // there is no empty frame available.
    // Therefore, createPage should return nullptr.
    Page *p2 = bm.createPage();
    assert(p2 == nullptr);
    cout << "As expected, no empty frame is available (createPage returned nullptr)." << endl;

    cout << "Test no empty frame passed." << endl;
}

int main()
{
    // Remove the test database file if it exists.
    std::remove(TEST_DB_FILE.c_str());

    cout << endl << "### Running BufferManager Tests ###" << endl;
    testCreateAndGetPage();

    std::remove(TEST_DB_FILE.c_str());

    cout << endl << "### Running BufferManager Interleaved Test ###" << endl;
    testInterleavedInsertAndQuery();

    std::remove(TEST_DB_FILE.c_str());
    testNoEmptyFrame();

    cout << endl << "All BufferManager tests passed!" << endl;
    return 0;
}