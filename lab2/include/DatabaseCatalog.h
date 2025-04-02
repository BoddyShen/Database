#ifndef DATABASECATALOG_H
#define DATABASECATALOG_H

#include "BTreeBase.h"
#include <string>
#include <unordered_map>

class DatabaseCatalog
{
  public:
    // Structure for table metadata
    struct TableInfo {
        std::string tableName;
        std::string filePath;
    };

    // Structure for index metadata
    struct IndexInfo {
        std::string indexName;
        std::string tableName; // The table the index is built on
        std::string filePath;
        std::string keyName; // The search attribute name
        BTreeBase *instance; // The B+ tree instance

        IndexInfo() : instance(nullptr) {}
    };

    // Adds a table entry to the catalog.
    void addTable(const TableInfo &table);

    // Retrieves table metadata by table name.
    TableInfo getTable(const std::string &tableName) const;

    // Adds an index entry to the catalog.
    void addIndex(const IndexInfo &index);

    // Retrieves index metadata by index name.
    IndexInfo getIndex(const std::string &indexName) const;

    // Check if the table exists in the catalog.
    bool tableExists(const std::string &tableName) const
    {
        return tables.find(tableName) != tables.end();
    }

    // Check if the index exists in the catalog.
    bool indexExists(const std::string &indexName) const
    {
        return indexes.find(indexName) != indexes.end();
    }

  private:
    std::unordered_map<std::string, TableInfo> tables;
    std::unordered_map<std::string, IndexInfo> indexes;
};

#endif // DATABASECATALOG_H