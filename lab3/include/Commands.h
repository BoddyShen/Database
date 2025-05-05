#ifndef COMMANDS_H
#define COMMANDS_H

#include <string>

// pre-processing step: load tsv file to the database, and create the index if needed
void pre_process(bool test = false);

// Execute the director-lookup query with title in [start,end],
// using buffer_size pages in your BufferManager.
void run_query(const std::string &start_range, const std::string &end_range, int buffer_size,
               bool test = false, double *selectivity = nullptr, int *join1tuples = nullptr,
               int *ioCount = nullptr);

#endif // COMMANDS_H