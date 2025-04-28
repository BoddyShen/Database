#include "Page.h"
#include "Row.h"
#include "Utilities.h"
#include <cassert>
#include <iostream>
#include <string>

using namespace std;

// Test case 1: Test inserting the first row and retrieving it
void testInsertAndRetrieveFirstRow()
{
    Page<MovieRow> *page = new Page<MovieRow>();
    int rowId = page->insertRow(Row("id1", "title1"));
    assert(rowId == 0);
    Row *r = page->getRow(0);
    assert(r != nullptr);
    string rowStr = Utilities::rowToString(*r);
    assert(rowStr == "Movie ID: id1, Title: title1");
    delete r;
    delete page;
    cout << "testInsertAndRetrieveFirstRow passed!" << endl;
}

// Test case 2: Test retrieving a non-existent row
void testRetrieveNonExistentRow()
{
    Page<MovieRow> *page = new Page<MovieRow>();
    // Since no data has been inserted, retrieving index 0 should return nullptr
    assert(page->getRow(0) == nullptr);
    delete page;
    cout << "testRetrieveNonExistentRow passed!" << endl;
}

// Test case 3: Test inserting multiple rows
void testInsertMultipleRows()
{
    Page<MovieRow> *page = new Page<MovieRow>();
    int rowId1 = page->insertRow(Row("id1", "title1"));
    int rowId2 = page->insertRow(Row("id2", "title2"));
    assert(rowId1 == 0);
    assert(rowId2 == 1);

    Row *r1 = page->getRow(1);
    assert(r1 != nullptr);
    string rowStr = Utilities::rowToString(*r1);
    assert(rowStr == "Movie ID: id2, Title: title2");
    delete r1;

    // Check that index 2 does not exist
    assert(page->getRow(2) == nullptr);

    delete page;
    cout << "testInsertMultipleRows passed!" << endl;
}

// Test case 4: Test inserting a row into a full page
void testInsertIntoFullPage()
{
    Page<MovieRow> *page = new Page<MovieRow>();
    // Fill the page with rows until it is full
    while (!page->isFull()) {
        page->insertRow(Row("id", "title"));
    }
    // Attempt to insert another row should return -1
    int rowId = page->insertRow(Row("id", "title"));
    assert(rowId == -1);
    delete page;
    cout << "testInsertIntoFullPage passed!" << endl;
}

// Test case 5: Test retrieving a row with a negative index
void testRetrieveNegativeIndex()
{
    Page<MovieRow> *page = new Page<MovieRow>();
    page->insertRow(Row("id1", "title1"));
    assert(page->getRow(-1) == nullptr);
    delete page;
    cout << "testRetrieveNegativeIndex passed!" << endl;
}

// Test case 6: Test retrieving a row with an out-of-bounds index
void testRetrieveOutOfBoundsIndex()
{
    Page<MovieRow> *page = new Page<MovieRow>();
    page->insertRow(Row("id1", "title1"));
    assert(page->getRow(1) == nullptr); // Only one row inserted, index 1 is out of bounds
    delete page;
    cout << "testRetrieveOutOfBoundsIndex passed!" << endl;
}

int main()
{
    cout << "### Running Page Tests ###" << endl;
    testInsertAndRetrieveFirstRow();
    testRetrieveNonExistentRow();
    testInsertMultipleRows();
    testInsertIntoFullPage();
    testRetrieveNegativeIndex();
    testRetrieveOutOfBoundsIndex();
    cout << "All test cases passed!" << endl;
    return 0;
}