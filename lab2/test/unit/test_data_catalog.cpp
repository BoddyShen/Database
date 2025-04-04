#include "DatabaseCatalog.h"
#include <cassert>
#include <iostream>

using namespace std;

// Test cases for data catalog
void test_data_catalog()
{
    // Create an empty catalog.
    DatabaseCatalog catalog;

    // Test adding and retrieving a table.
    DatabaseCatalog::TableInfo moviesTable;
    moviesTable.tableName = "Movies";
    moviesTable.filePath = "movies.bin";
    catalog.addTable(moviesTable);

    // Check that the table exists.
    assert(catalog.tableExists("Movies") && "Movies table should exist in catalog.");
    // Check that a non-existent table returns false.
    assert(!catalog.tableExists("NonExistent") && "NonExistent table should not exist.");

    // Retrieve table info and verify its contents.
    DatabaseCatalog::TableInfo retrievedTable = catalog.getTable("Movies");
    assert(retrievedTable.tableName == "Movies");
    assert(retrievedTable.filePath == "movies.bin");

    // Test adding and retrieving an index.
    DatabaseCatalog::IndexInfo titleIndex;
    titleIndex.indexName = "title_index.bin";
    titleIndex.tableName = "Movies";
    titleIndex.filePath = "title_index.bin";
    titleIndex.keyName = "title";
    catalog.addIndex(titleIndex);

    // Check that the index exists.
    assert(catalog.indexExists("title_index.bin") && "Title index should exist in catalog.");
    // Check that a non-existent index returns false.
    assert(!catalog.indexExists("nonexistent_index.bin") && "Non-existent index should not exist.");

    // Retrieve index info and verify its contents.
    DatabaseCatalog::IndexInfo retrievedIndex = catalog.getIndex("title_index.bin");
    assert(retrievedIndex.indexName == "title_index.bin");
    assert(retrievedIndex.tableName == "Movies");
    assert(retrievedIndex.filePath == "title_index.bin");
    assert(retrievedIndex.keyName == "title");

    cout << "All DatabaseCatalog tests passed." << endl;
}

int main() { test_data_catalog(); }