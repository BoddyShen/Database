#include "Commands.h"
#include <iostream>

using namespace std;

void pre_process() { cout << "Start Pre-processing." << endl; }

void run_query(const std::string &start_range, const std::string &end_range, int buffer_size)
{
    cout << "Start Querying." << endl;
    cout << "Start range: " << start_range << endl;
    cout << "End range: " << end_range << endl;
    cout << "Buffer size: " << buffer_size << endl;
}