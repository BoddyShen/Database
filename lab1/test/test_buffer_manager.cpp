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
    bm.printStatus();
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
    bm.printStatus();
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

int main()
{
    std::remove(TEST_DB_FILE.c_str());

    cout << "### Running BufferManager Tests ###" << endl;
    testCreateAndGetPage();
    cout << "All BufferManager tests passed!" << endl;
    return 0;
}