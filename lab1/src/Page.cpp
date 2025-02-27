#include "Page.h"
#include <cstring>

using namespace std;

Page::Page() : numRecords(0) { pageData.fill(0); }

Row *Page::getRow(int rowId)
{
    if (rowId < 0 || rowId >= numRecords) {
        return nullptr;
    }
    size_t offset = rowId * RECORD_SIZE;
    Row *row = new Row();
    // Copy the fixed-length fields from the data buffer into the new Row.
    memcpy(row->movieId.data(), pageData.data() + offset, MOVIE_ID_SIZE);
    memcpy(row->title.data(), pageData.data() + offset + MOVIE_ID_SIZE, TITLE_SIZE);
    return row;
}

int Page::insertRow(const Row &row)
{
    if (isFull()) {
        return -1;
    }
    size_t offset = numRecords * RECORD_SIZE;
    // Copy movieId and title from the input row into the page's data buffer.
    memcpy(pageData.data() + offset, row.movieId.data(), MOVIE_ID_SIZE);
    memcpy(pageData.data() + offset + MOVIE_ID_SIZE, row.title.data(), TITLE_SIZE);
    numRecords++;
    return numRecords - 1;
}

bool Page::isFull()
{
    // Checks if adding another record would exceed the page's capacity.
    return ((numRecords + 1) * RECORD_SIZE) > MAX_PAGE_SIZE;
}