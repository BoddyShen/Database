#ifndef PAGE_H
#define PAGE_H

#include "Constants.h"
#include "Row.h"

template <typename RowType> class Page
{
  public:
    static constexpr size_t HEADER_SIZE = sizeof(int);
    static constexpr size_t RECORD_SIZE = sizeof(RowType);

    // Constructor
    Page();

    /**
     * Fetches a row from the page by its row ID.
     * @param rowId The ID of the row to retrieve.
     * @return A pointer to the Row object containing the requested data,
     *         or nullptr if the row does not exist.
     */
    RowType *getRow(int rowId) const;

    /**
     * Inserts a new row into the page.
     * @param row The Row object containing the data to insert.
     * @return The row ID of the inserted row, or -1 if the page is full.
     */
    int insertRow(const RowType &row);

    /**
     * Checks if the page is full.
     * @return true if the page is full, false otherwise.
     */
    bool isFull() const;

    int getPid() const;

    void setPid(int pid);

    int getNumRecords() { return numRecords; }

    void setNumRecords(int num) { numRecords = num; }

    uint8_t *getPageData() { return pageData.data(); }

  private:
    // Fixed-size data buffer (4 KB) for the packed records.
    std::array<uint8_t, MAX_PAGE_SIZE> pageData;
    // The number of rows currently stored in the page.
    int numRecords = 0;
    int pageId;
};

template <typename RowType> Page<RowType>::Page() { pageData.fill(0); }

template <typename RowType> RowType *Page<RowType>::getRow(int rowId) const
{
    if (rowId < 0 || rowId >= numRecords) return nullptr; // Invalid rowId
    RowType *row = new RowType();
    size_t off = HEADER_SIZE + rowId * RECORD_SIZE;
    std::memcpy(row, pageData.data() + off, RECORD_SIZE);
    return row;
}

template <typename RowType> int Page<RowType>::insertRow(const RowType &row)
{
    // First 4 bytes of the pageData is the number of records in the page.
    // Check if the page has enough space to insert a new record.
    if (HEADER_SIZE + (numRecords + 1) * RECORD_SIZE > MAX_PAGE_SIZE) return -1;
    auto offset = HEADER_SIZE + numRecords * RECORD_SIZE;
    std::memcpy(pageData.data() + offset, &row, RECORD_SIZE);
    ++numRecords;
    std::memcpy(pageData.data(), &numRecords, HEADER_SIZE);
    return numRecords - 1;
}

template <typename RowType> bool Page<RowType>::isFull() const
{
    // Checks if adding another record would exceed the page's capacity.
    return HEADER_SIZE + (numRecords + 1) * RECORD_SIZE > MAX_PAGE_SIZE;
}

template <typename RowType> int Page<RowType>::getPid() const { return pageId; }

template <typename RowType> void Page<RowType>::setPid(int pid) { pageId = pid; }

#endif // PAGE_H