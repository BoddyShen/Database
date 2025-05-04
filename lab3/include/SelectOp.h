#ifndef SELECTOP_H
#define SELECTOP_H

#include "Operator.h"
#include <functional>

// Filters a single input column (picked by idx) against a predicate.
// SelectOperator usually be used on top of ScanOperator or IndexOperator, which as child in input.
class SelectOp : public Operator
{
  public:
    SelectOp(Operator *child, std::function<bool(const Tuple &)> predicate)
        : child(child), predicate(predicate), selected_num(0), total_num(0)
    {
    }

    void open() override { child->open(); }

    bool next(Tuple &out) override
    {
        while (child->next(out)) {
            total_num++;
            if (predicate(out)) {
                selected_num++;
                return true;
            }
        }
        return false;
    }

    void close() override { child->close(); }

    double getSelectivity() {
        if (total_num == 0) {
            return 0.0;
        }
        return static_cast<double>(selected_num) / total_num;
    }

  private:
    Operator *child;
    std::function<bool(const Tuple &)> predicate;
    int selected_num;
    int total_num;
};
#endif // SELECTOP_H