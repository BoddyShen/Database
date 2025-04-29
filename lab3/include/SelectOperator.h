#ifndef SELECTOPERATOR_H
#define SELECTOPERATOR_H

#include "Operator.h"
#include <functional>

// Filters a single input column (picked by idx) against a predicate.
// SelectOperator usually be used on top of ScanOperator or IndexOperator, which as child in input.
class SelectOperator : public Operator
{
  public:
    SelectOperator(Operator *child, std::function<bool(const Tuple &)> predicate)
        : child(child), predicate(predicate)
    {
    }

    void open() override { child->open(); }

    bool next(Tuple &out) override
    {
        while (child->next(out)) {
            if (predicate(out)) {
                return true;
            }
        }
        return false;
    }

    void close() override { child->close(); }

  private:
    Operator *child;
    std::function<bool(const Tuple &)> predicate;
};
#endif // SELECTOPERATOR_H