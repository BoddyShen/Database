#ifndef COMMANDS_H
#define COMMANDS_H

#include <string>

// pre-processing step: load CSVs, build heapfiles/indexes, etc.
void pre_process();

// Execute the director-lookup query with title in [start,end],
// using buffer_size pages in your BufferManager.
void run_query(const std::string &start_range, const std::string &end_range, int buffer_size);

#endif // COMMANDS_H