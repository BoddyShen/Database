#include "DatabaseCatalog.h"
#include <stdexcept>

// Add a table entry to the catalog.
void DatabaseCatalog::addTable(const TableInfo &table) { tables[table.tableName] = table; }

// Retrieve table metadata by table name.
DatabaseCatalog::TableInfo DatabaseCatalog::getTable(const std::string &tableName) const
{
    auto it = tables.find(tableName);
    if (it != tables.end()) {
        return it->second;
    }
    throw std::runtime_error("Table not found: " + tableName);
}

// Add an index entry to the catalog.
void DatabaseCatalog::addIndex(const IndexInfo &index) { indexes[index.indexName] = index; }

// Retrieve index metadata by index name.
DatabaseCatalog::IndexInfo DatabaseCatalog::getIndex(const std::string &indexName) const
{
    auto it = indexes.find(indexName);
    if (it != indexes.end()) {
        return it->second;
    }
    throw std::runtime_error("Index not found: " + indexName);
}
