#ifndef PAGE_H
#define PAGE_H

#include "Constants.h"
#include "Row.h"

class Page
{
  public:
    /**
     * Fetches a row from the page by its row ID.
     * @param rowId The ID of the row to retrieve.
     * @return A pointer to the Row object containing the requested data,
     *         or nullptr if the row does not exist.
     */
    Row *getRow(int rowId);

    /**
     * Inserts a new row into the page.
     * @param row The Row object containing the data to insert.
     * @return The row ID of the inserted row, or -1 if the page is full.
     */
    int insertRow(const Row &row);

    /**
     * Checks if the page is full.
     * @return true if the page is full, false otherwise.
     */
    bool isFull();
};

#endif // PAGE_H