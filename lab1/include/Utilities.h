#ifndef UTILITIES_H
#define UTILITIES_H

#include "BufferManager.h"
#include <string>

class Utilities
{
  public:
    // Loads the buffer manager with the IMDb dataset.
    static void loadDataset(BufferManager &bf, const std::string &filepath);
};

#endif // UTILITIES_H