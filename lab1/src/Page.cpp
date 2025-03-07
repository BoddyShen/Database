#include "Page.h"
#include <cstring>

using namespace std;

Page::Page() : numRecords(0) { pageData.fill(0); }

Row *Page::getRow(int rowId)
{
    if (rowId < 0 || rowId >= numRecords) return nullptr;
    size_t offset = sizeof(int) + rowId * RECORD_SIZE;
    Row *row = new Row();
    memcpy(row->movieId.data(), pageData.data() + offset, MOVIE_ID_SIZE);
    memcpy(row->title.data(), pageData.data() + offset + MOVIE_ID_SIZE, TITLE_SIZE);
    return row;
}

int Page::insertRow(const Row &row)
{
    // First 4 bytes of the pageData is the number of records in the page.
    // Check if the page has enough space to insert a new record.
    if (((numRecords + 1) * RECORD_SIZE + sizeof(int)) > MAX_PAGE_SIZE) {
        return -1;
    }
    size_t offset = sizeof(int) + numRecords * RECORD_SIZE;
    memcpy(pageData.data() + offset, row.movieId.data(), MOVIE_ID_SIZE);
    memcpy(pageData.data() + offset + MOVIE_ID_SIZE, row.title.data(), TITLE_SIZE);
    numRecords++;
    std::memcpy(pageData.data(), &numRecords, sizeof(numRecords));
    return numRecords - 1;
}

bool Page::isFull()
{
    // Checks if adding another record would exceed the page's capacity.
    return ((sizeof(int) + (numRecords + 1) * RECORD_SIZE) > MAX_PAGE_SIZE);
}

int Page::getPid() { return pageId; }

void Page::setPid(int pid) { pageId = pid; }