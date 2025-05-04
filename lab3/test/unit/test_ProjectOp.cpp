#include "Operator.h"
#include "ProjectOp.h" 
#include "MockScanOp.h" 
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <utility> 

std::vector<Tuple> getAllTuples(Operator* op) {
    std::vector<Tuple> allTuples;
    op->open();
    Tuple tempTuple;
    int count = 0;
    while (op->next(tempTuple)) {
        allTuples.push_back(tempTuple);
        count++;
    }
    op->close();
    std::cout << "[Info] Finished reading " << count << " tuples." << std::endl;
    return allTuples;
}

std::vector<Tuple> createMockData() {
    std::vector<Tuple> data;
    data.push_back({{"t01", "Carmencita"}});
    data.push_back({{"t02", "Le clown et ses chiens"}});
    data.push_back({{"t03", "Pauvre Pierrot"}});
    data.push_back({{"t04", "Un bon bock"}});
    data.push_back({{"t05", "Blacksmith Scene"}});
    data.push_back({{"t06", "Chinese Opium Den"}});
    data.push_back({{"t07", "Corbett and Courtney Before the Kinetograph"}});
    data.push_back({{"t08", "Edison Kinetoscopic Record of a Sneeze"}});
    data.push_back({{"t09", "Miss Jerry"}});
    data.push_back({{"t10", "Le Voyage dans la Lune"}});
    return data;
}


void testProjectSingleColumn() {
    std::cout << "\n--- Testing ProjectOp (Single Column: Title Index 1) ---" << std::endl;
    auto mockTuples = createMockData();
    MockScanOp mockScanner(mockTuples);
    std::vector<int> keepCols = {1}; 
    ProjectOp projectOp(&mockScanner, keepCols);
    std::vector<Tuple> results = getAllTuples(&projectOp);

    assert(results.size() == mockTuples.size());
    std::cout << "[Info] Expected fields per tuple: 1" << std::endl;
    for (size_t i = 0; i < results.size(); ++i) {
        // std::cout << results[i].fields[0] << " results from op" << std::endl;
        assert(results[i].fields.size() == 1);
        assert(results[i].fields[0] == mockTuples[i].fields[1]);
    }
    std::cout << "[Check] Verified field count and content for all projected tuples." << std::endl;
    std::cout << "--- Test ProjectOp (Single Column: Title) PASSED ---" << std::endl;
}

void testProjectNoColumns() {
    std::cout << "\n--- Testing ProjectOp (No Columns) ---" << std::endl;
    auto mockTuples = createMockData();
    MockScanOp mockScanner(mockTuples);
    std::vector<int> keepCols = {}; 
    ProjectOp projectOp(&mockScanner, keepCols);
    std::vector<Tuple> results = getAllTuples(&projectOp);

    assert(results.size() == mockTuples.size());
    std::cout << "[Info] Expected fields per tuple: 0" << std::endl;
    for (const auto& tuple : results) {
        assert(tuple.fields.empty());
    }
    std::cout << "[Check] Verified all projected tuples have zero fields." << std::endl;
    std::cout << "--- Test ProjectOp (No Columns) PASSED ---" << std::endl;
}


int main() {
    std::cout << "===== Starting ProjectOp Tests with Mock Data =====" << std::endl;

    try {
        testProjectSingleColumn();
        testProjectNoColumns();

        std::cout << "\n>>> ALL ProjectOp TESTS using Mock Data PASSED <<<" << std::endl; 
        return 0;
    } catch (const std::exception &e) {
        std::cerr << "\n*** TEST FAILED (Standard exception): " << e.what() << " ***" << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "\n*** TEST FAILED (Unknown exception) ***" << std::endl;
        return 1;
    }
}