#include "../../include/BNLJoinOp.h"
#include "../../include/BufferManager.h"
#include "../../include/Operator.h"
#include "MockScanOp.h"
#include <cassert>
#include <iostream>
#include <vector>

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

// Mock data creation for testing
std::vector<Tuple> createLeftMockData() {
    return {
        {{"l01", "Alice"}},
        {{"l02", "Bob"}},
        {{"l03", "Charlie"}},
        {{"l04", "David"}}
    };
}

std::vector<Tuple> createRightMockData() {
    return {
        {{"r01", "Alice", "Engineer"}},
        {{"r02", "Bob", "Doctor"}},
        {{"r03", "Eve", "Artist"}},
        {{"r04", "Charlie", "Teacher"}}
    };
}

// Test case for BNLJoinOp
void testBNLJoinOp() {
    std::cout << "\n--- Testing BNLJoinOp ---" << std::endl;

    auto leftTuples = createLeftMockData();
    auto rightTuples = createRightMockData();

    MockScanOp leftScanner(leftTuples);
    MockScanOp rightScanner(rightTuples);

    // Define key extractors for join
    auto leftKeyExtractor = [](const Tuple &t) { return t.fields[1]; };
    auto rightKeyExtractor = [](const Tuple &t) { return t.fields[1]; };

    // Create BNLJoinOp instance
    BufferManager bufferManager(20); 
    std::unordered_map<std::string, int> idx_map = {{"movieId", 0}, {"title", 1}};
    BNLJoinOp<std::string, MovieRow> joinOp(
        bufferManager, &leftScanner, &rightScanner, 2, "temp_block_file",
        leftKeyExtractor, rightKeyExtractor, idx_map);

    // Get all joined tuples
    std::vector<Tuple> results = getAllTuples(&joinOp);

    // Expected results: Join on names "Alice", "Bob", and "Charlie"
    std::cout << "[Info] Expected joined tuples: 3" << std::endl;
    std::cout << "[Info] Actual joined tuples:   " << results.size() << std::endl;
    assert(results.size() == 3);

    // Verify the joined tuples
    for (const auto &tuple : results) {
        assert(tuple.fields.size() == 5); // Joined tuple should have 4 fields
        if (tuple.fields[0] == "l01") {
            assert(tuple.fields[1] == "Alice");
            assert(tuple.fields[2] == "r01");
            assert(tuple.fields[4] == "Engineer");
        } else if (tuple.fields[0] == "l02") {
            assert(tuple.fields[1] == "Bob");
            assert(tuple.fields[2] == "r02");
            assert(tuple.fields[4] == "Doctor");
        } else if (tuple.fields[0] == "l03") {
            assert(tuple.fields[1] == "Charlie");
            assert(tuple.fields[2] == "r04");
            assert(tuple.fields[4] == "Teacher");
        } else {
            assert(false); // Unexpected tuple
        }
    }

    std::cout << "--- Test BNLJoinOp PASSED ---" << std::endl;
}

int main() {
    std::cout << "===== Starting BNLJoinOp Tests =====" << std::endl;

    try {
        testBNLJoinOp();

        std::cout << "\n>>> ALL BNLJoinOp TESTS PASSED <<<" << std::endl;
        return 0;
    } catch (const std::exception &e) {
        std::cerr << "\n*** TEST FAILED (Standard exception): " << e.what() << " ***" << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "\n*** TEST FAILED (Unknown exception) ***" << std::endl;
        return 1;
    }
}