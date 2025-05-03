#include "Operator.h"
#include "SelectOp.h" 
#include "MockScanOp.h"
#include <cassert>
#include <functional> 
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

std::vector<Tuple> getAllTuples(Operator* op, const std::string& opName = "Operator") {
    std::vector<Tuple> allTuples;
    std::cout << "[Info] Opening " << opName << "..." << std::endl;
    op->open();
    Tuple tempTuple;
    int count = 0;
    std::cout << "[Info] Reading tuples from " << opName << "..." << std::endl;
    while (op->next(tempTuple)) {
        allTuples.push_back(tempTuple);
        count++;
    }
     std::cout << "[Info] Finished reading " << count << " tuples from " << opName << "." << std::endl;
    op->close();
    std::cout << "[Info] Closed " << opName << "." << std::endl;
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

void testSelectSpecificMovie() {
    std::cout << "\n--- Testing SelectOp (Specific Movie ID 't05') ---" << std::endl;
    auto mockTuples = createMockData();
    MockScanOp mockScanner(mockTuples);

    const std::string target_tconst = "t05";
    auto predicate = [&target_tconst](const Tuple &t) {
        return t.fields.size() > 0 && t.fields[0] == target_tconst;
    };
    SelectOp selectOp(&mockScanner, predicate);

    std::vector<Tuple> results = getAllTuples(&selectOp, "SelectOp (Specific ID)");

    std::cout << "[Info] Expected selected tuples: 1" << std::endl;
    std::cout << "[Info] Actual selected tuples:   " << results.size() << std::endl;
    assert(results.size() == 1);

    assert(!results.empty() && results[0].fields.size() >= 2);
    assert(results[0].fields[0] == target_tconst);
    assert(results[0].fields[1] == "Blacksmith Scene"); 
    std::cout << "[Check] Correct movie selected based on ID." << std::endl;

    
    assert(mockTuples[4].fields[0] == "t05");

    std::cout << "--- Test SelectOp (Specific Movie ID) PASSED ---" << std::endl;
}


void testSelectMoviesStartingWithLe() {
    std::cout << "\n--- Testing SelectOp (Title starts with 'Le ') ---" << std::endl;
    auto mockTuples = createMockData();
    MockScanOp mockScanner(mockTuples);

    const std::string prefix = "Le ";
    auto predicate = [&prefix](const Tuple &t) {
        return t.fields.size() > 1 && t.fields[1].rfind(prefix, 0) == 0;
    };
    SelectOp selectOp(&mockScanner, predicate);

    
    std::vector<Tuple> results = getAllTuples(&selectOp, "SelectOp (Le Prefix)");

    std::cout << "[Info] Expected selected tuples: 2" << std::endl;
    std::cout << "[Info] Actual selected tuples:   " << results.size() << std::endl;
    assert(results.size() == 2);

    
    bool found_t02 = false;
    bool found_t10 = false;
    for (const auto &tuple : results) {
        assert(tuple.fields.size() > 1);
        assert(tuple.fields[1].rfind(prefix, 0) == 0);
        if (tuple.fields[0] == "t02") {
            assert(tuple.fields[1] == "Le clown et ses chiens");
            found_t02 = true;
        } else if (tuple.fields[0] == "t10") {
             assert(tuple.fields[1] == "Le Voyage dans la Lune");
             found_t10 = true;
        } else {
            assert(false); 
        }
    }
    assert(found_t02 && found_t10); 
    std::cout << "[Check] Verified predicate and content for all selected tuples." << std::endl;

    std::cout << "--- Test SelectOp (Title starts with 'Le ') PASSED ---" << std::endl;
}


void testSelectNoResults() {
    std::cout << "\n--- Testing SelectOp (No Results Expected) ---" << std::endl;

    auto mockTuples = createMockData();
    MockScanOp mockScanner(mockTuples);

    auto predicate = [](const Tuple &t) {
        return t.fields.size() > 0 && t.fields[0] == "THIS_ID_DOES_NOT_EXIST";
    };
    SelectOp selectOp(&mockScanner, predicate);

    std::vector<Tuple> results = getAllTuples(&selectOp, "SelectOp (No Results)");

    std::cout << "[Info] Expected selected tuples: 0" << std::endl;
    std::cout << "[Info] Actual selected tuples:   " << results.size() << std::endl;
    assert(results.empty());

    std::cout << "--- Test SelectOp (No Results Expected) PASSED ---" << std::endl;
}


int main() {
    std::cout << "===== Starting SelectOp Tests with Mock Data =====" << std::endl;

    try {
        testSelectSpecificMovie();
        testSelectMoviesStartingWithLe();
        testSelectNoResults();

        std::cout << "\n>>> ALL SelectOp TESTS using Mock Data PASSED <<<" << std::endl;
        return 0;
    } catch (const std::exception &e) {
        std::cerr << "\n*** TEST FAILED (Standard exception): " << e.what() << " ***" << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "\n*** TEST FAILED (Unknown exception) ***" << std::endl;
        return 1;
    }
}