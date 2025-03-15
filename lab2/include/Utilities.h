#ifndef UTILITIES_H
#define UTILITIES_H

#include "BufferManager.h"
#include "Row.h"
#include <string>

class Utilities
{
  public:
    // Loads the buffer manager with the IMDb dataset.
    static void loadDataset(BufferManager &bf, const std::string &filepath);

    static std::string rowToString(const Row &row);
};

#endif // UTILITIES_H