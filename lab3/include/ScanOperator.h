#ifndef SCANOPERATOR_H
#define SCANOPERATOR_H

#include "BufferManager.h"
#include "Operator.h"
#include "Page.h"
#include <string>

// ScanOperator can be used to scan a file and return tuples one by one
// bm: BufferManager instance to manage buffer pages
// filePath: path to the file to be scanned
// startPid: page id to start scanning from
template <typename RowType> class ScanOperator : public Operator
{
  public:
    ScanOperator(BufferManager &bm, const std::string &filePath, int startPid = 0)
        : bm(bm), filePath(filePath), pid(startPid)
    {
    }

    void open() override { page = bm.getPage<RowType>(pid++, filePath); }

    bool next(Tuple &out) override
    {
        if (!page) return false;
        RowType *r = page->getRow(slot++);
        if (!r) {
            // end of this page
            bm.unpinPage(page->getPid(), filePath);
            page = bm.getPage<RowType>(pid++, filePath);
            slot = 0;
            if (!page || page->getNumRecords() == 0) return false;
            r = page->getRow(slot++);
        }
        // flatten RowType into Tuple:
        out.fields.clear();
        // extract fields from RowType to Tuple
        out = r->toTuple();
        delete r;
        return true;
    }

    void close() override
    {
        if (page) {
            bm.unpinPage(page->getPid(), filePath);
            page = nullptr;
        }
    }

  private:
    BufferManager &bm;
    std::string filePath;
    Page<RowType> *page = nullptr;
    int pid, slot = 0;
};

#endif // SCANOPERATOR_H