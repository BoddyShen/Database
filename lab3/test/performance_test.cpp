#include "../include/Commands.h"
#include <iostream>
#include <fstream>

int main() {
    std::ofstream out("performance_test.csv");
    int bufferSize = 20;
    double selectivity = 0.0;
    int ioCount = 0;
    out << "selectivity,IOCount\n";
    for (char c = 'B'; c <= 'Z'; ++c) {
        std::string start_range(1, 'A');
        std::string end_range(1, c);
        run_query(start_range, end_range, bufferSize, true, &selectivity, &ioCount);
        out << selectivity << "," << ioCount << "\n";
    }
}