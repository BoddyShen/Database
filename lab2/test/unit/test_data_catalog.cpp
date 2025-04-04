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

// Test multiple tables and indexes
void test_multiple_entries()
{
    cout << "Testing multiple tables and indexes..." << endl;
    DatabaseCatalog catalog;

    // Add multiple tables
    DatabaseCatalog::TableInfo moviesTable;
    moviesTable.tableName = "Movies";
    moviesTable.filePath = "movies.bin";
    catalog.addTable(moviesTable);

    DatabaseCatalog::TableInfo actorsTable;
    actorsTable.tableName = "Actors";
    actorsTable.filePath = "actors.bin";
    catalog.addTable(actorsTable);

    // Add multiple indexes for the same table
    DatabaseCatalog::IndexInfo titleIndex;
    titleIndex.indexName = "title_index.bin";
    titleIndex.tableName = "Movies";
    titleIndex.filePath = "title_index.bin";
    titleIndex.keyName = "title";
    catalog.addIndex(titleIndex);

    DatabaseCatalog::IndexInfo yearIndex;
    yearIndex.indexName = "year_index.bin";
    yearIndex.tableName = "Movies";
    yearIndex.filePath = "year_index.bin";
    yearIndex.keyName = "year";
    catalog.addIndex(yearIndex);

    // Verify all entries exist
    assert(catalog.tableExists("Movies"));
    assert(catalog.tableExists("Actors"));
    assert(catalog.indexExists("title_index.bin"));
    assert(catalog.indexExists("year_index.bin"));

    cout << "Multiple entries test passed." << endl;
}

// Test error handling
void test_error_handling()
{
    cout << "Testing error handling..." << endl;
    DatabaseCatalog catalog;

    // Test retrieving non-existent table
    bool exceptionCaught = false;
    try {
        auto table = catalog.getTable("NonExistentTable");
    } catch (const runtime_error& e) {
        exceptionCaught = true;
        cout << "Successfully caught exception for non-existent table: " << e.what() << endl;
    }
    assert(exceptionCaught && "Expected exception for non-existent table");

    // Add a table
    DatabaseCatalog::TableInfo testTable;
    testTable.tableName = "TestTable";
    testTable.filePath = "test.bin";
    catalog.addTable(testTable);

    // Test retrieving non-existent index
    exceptionCaught = false;
    try {
        auto index = catalog.getIndex("nonexistent_index.bin");
    } catch (const runtime_error& e) {
        exceptionCaught = true;
        cout << "Successfully caught exception for non-existent index: " << e.what() << endl;
    }
    assert(exceptionCaught && "Expected exception for non-existent index");

    cout << "Error handling test passed." << endl;
}

int main() 
{
    test_data_catalog();
    test_multiple_entries();
    test_error_handling();
    cout << "All DatabaseCatalog tests passed!" << endl;
    return 0;
}