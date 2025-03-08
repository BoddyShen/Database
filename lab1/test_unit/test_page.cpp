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
    Page *page = new Page();
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
    Page *page = new Page();
    // Since no data has been inserted, retrieving index 0 should return nullptr
    assert(page->getRow(0) == nullptr);
    delete page;
    cout << "testRetrieveNonExistentRow passed!" << endl;
}

// Test case 3: Test inserting multiple rows
void testInsertMultipleRows()
{
    Page *page = new Page();
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

int main()
{
    cout << "### Running Page Tests ###" << endl;
    testInsertAndRetrieveFirstRow();
    testRetrieveNonExistentRow();
    testInsertMultipleRows();
    cout << "All test cases passed!" << endl;
    return 0;
}