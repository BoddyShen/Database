#ifndef PROJECTOP_H
#define PROJECTOP_H

#include "Operator.h"
#include <vector>

// Picks a subset of columns from its child.
class ProjectOp : public Operator
{
  public:
    ProjectOp(Operator *child, std::vector<int> keepCols)
        : child(child), keepCols(std::move(keepCols))
    {
    }

    void open() override { child->open(); }

    bool next(Tuple &out) override
    {
        Tuple in;
        if (!child->next(in)) return false;
        out.fields.clear();
        for (int c : keepCols) {
            out.fields.push_back(in.fields[c]);
        }
        return true;
    }

    void close() override { child->close(); }

  private:
    Operator *child;
    std::vector<int> keepCols;
};

#endif // PROJECTOP_H