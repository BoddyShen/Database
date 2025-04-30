#ifndef WORKED_ON_MATERIAL_OP_H
#define WORKED_ON_MATERIAL_OP_H

#include "Operator.h"
#include "Row.h"
#include "ScanOp.h"

// Materializes the data from the child operator into a temporary file.
class WorkedOnMaterialOp : public Operator
{
    Operator *child;
    BufferManager &bm;
    std::string tempFile;
    bool materialized = false;
    ScanOp<WorkedOnKeyRow> *scanOpPtr = nullptr;

  public:
    WorkedOnMaterialOp(Operator *child, BufferManager &bm, const std::string &tempFile)
        : child(child), bm(bm), tempFile(tempFile)
    {
    }

    void open() override { child->open(); }

    bool next(Tuple &out) override
    {
        if (!materialized) {
            // Materialize the data
            materialize();
            materialized = true;
            scanOpPtr = new ScanOp<WorkedOnKeyRow>(bm, tempFile);
            scanOpPtr->open();
        }
        return scanOpPtr->next(out);
    }

    void materialize()
    {
        if (materialized) return;
        bm.registerFile(tempFile);

        Page<WorkedOnKeyRow> *page = bm.createPage<WorkedOnKeyRow>(tempFile);
        int pid = page->getPid();
        Tuple in;
        int counter = 0;
        while (child->next(in)) {
            WorkedOnKeyRow row(in.fields[0], in.fields[1]);
            counter++;
            if (page->isFull()) {
                bm.unpinPage(pid, tempFile);
                page = bm.createPage<WorkedOnKeyRow>(tempFile);
                pid = page->getPid();
            }
            page->insertRow(row);
        }
        cout << "Materialized " << counter << " rows into " << tempFile << endl;
        bm.unpinPage(pid, tempFile);
    }

    void close() override
    {
        if (scanOpPtr) {
            scanOpPtr->close();
            delete scanOpPtr;
            scanOpPtr = nullptr;
            std::remove(tempFile.c_str());
        }
        child->close();
    }
};

#endif // WORKED_ON_MATERIAL_OP_H