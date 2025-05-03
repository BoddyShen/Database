#include "BufferManager.h" 
#include "Operator.h"
#include "Page.h"          
#include "Row.h"
#include "ScanOp.h"
#include <cassert>
#include <cstdio>
#include <fstream> 
#include <iostream>        
#include <string>          
#include <vector>
#include <stdexcept>

const std::string EXISTING_DATA_FILE = "movie100000.bin";
using TestRowType = MovieRow;


std::vector<Tuple> scanAllTuples(BufferManager &bm, const std::string &filePath) {
    std::vector<Tuple> allTuples;
    ScanOp<TestRowType> scanner(bm, filePath); // Start scan from page 0 by default

    scanner.open();
    Tuple tempTuple;
    std::cout << "[Info] Starting scan of file: " << filePath << std::endl;
    int count = 0;
    while (scanner.next(tempTuple)) {
        allTuples.push_back(tempTuple);
        count++;

    }
    scanner.close();
    std::cout << "[Info] Finished scan. Read " << count << " tuples." << std::endl;
    return allTuples;
}


void testScanFullFile(BufferManager &bm) {
    std::cout << "\n--- Testing Full File Scan (" << EXISTING_DATA_FILE << ") ---" << std::endl;
    std::vector<Tuple> results = scanAllTuples(bm, EXISTING_DATA_FILE);

    
    const size_t EXPECTED_TOTAL_TUPLE_COUNT = 100000 - 1;

    std::cout << "[Info] Expected total tuples: " << EXPECTED_TOTAL_TUPLE_COUNT << std::endl;
    std::cout << "[Info] Actual tuples found:   " << results.size() << std::endl;

    assert(results.size() == EXPECTED_TOTAL_TUPLE_COUNT);

    std::cout << "--- Test Full File Scan PASSED ---" << std::endl;
}

void testScanFirstTuples(BufferManager &bm) {
     std::cout << "\n--- Testing First Tuples Scan (" << EXISTING_DATA_FILE << ") ---" << std::endl;
    std::vector<Tuple> results = scanAllTuples(bm, EXISTING_DATA_FILE);

    assert(!results.empty()); 

    if (results.size() >= 1) {
        assert(results[0].fields.size() >= 2);
        assert(results[0].fields[0] == "tt0000001"); 
        assert(results[0].fields[1] == "Carmencita");
        std::cout << "[Check] Tuple 0 content matches expected (example)." << std::endl;
    }

    if (results.size() >= 2) {
        assert(results[1].fields.size() >= 2);
        assert(results[1].fields[0] == "tt0000002");
        assert(results[1].fields[1] == "Le clown et ses chiens");
        std::cout << "[Check] Tuple 1 content matches expected (example)." << std::endl;
    }

    std::cout << "--- Test First Tuples Scan PASSED ---" << std::endl;
}

int main()
{
    const int testBufferSize = 50;
    BufferManager bm(testBufferSize);

    std::ifstream testFileCheck(EXISTING_DATA_FILE);
    if (!testFileCheck) {
         std::cerr << "\n*** ERROR: Pre-existing data file not found: "
                   << EXISTING_DATA_FILE << " ***" << std::endl;
         std::cerr << "*** Please ensure the file exists and the path is correct. ***" << std::endl;
         return 1; 
    }
    testFileCheck.close();

    bool existed = bm.registerFile(EXISTING_DATA_FILE);
     if (!existed) {
         std::cout << "[Warning] BufferManager reported file did not exist, but it was found? Registering now." << std::endl;
     } else {
          std::cout << "[Info] Registered existing file with BufferManager: " << EXISTING_DATA_FILE << std::endl;
     }


    try
    {
        testScanFullFile(bm);
        testScanFirstTuples(bm);

        std::cout << "\n>>> ALL ScanOp TESTS using '" << EXISTING_DATA_FILE << "' PASSED <<<" << std::endl;
        return 0;
    }
    catch (...)
    {
        std::cerr << "\n*** TEST FAILED (Unknown exception) ***" << std::endl;
        return 1;
    }
}
