#ifndef OPERATOR_H
#define OPERATOR_H

#include "Row.h"

class Operator
{
  public:
    virtual ~Operator() = default;
    // Initializes the operator
    virtual void open() = 0;
    // Provide next tuple to parent operator
    virtual bool next(Tuple &out) = 0;
    // Release any resources held by the operator
    virtual void close() = 0;
};

#endif // OPERATOR_H