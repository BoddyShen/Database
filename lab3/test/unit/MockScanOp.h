#ifndef MOCKSCANOP_H
#define MOCKSCANOP_H

#include "Operator.h" 
#include <vector>
#include <iostream>

class MockScanOp : public Operator {
public:
    
    MockScanOp(const std::vector<Tuple>& data) : mockData(data), currentIndex(0) {
         std::cout << "[MockScanOp] Initialized with " << data.size() << " tuples." << std::endl;
    }

    void open() override {
        currentIndex = 0;
        std::cout << "[MockScanOp] Opened (reset index)." << std::endl;
    }

    bool next(Tuple& out) override {
        if (currentIndex < mockData.size()) {
            out = mockData[currentIndex]; 
            currentIndex++; 
            return true;
        } else {
            
            return false; 
        }
    }

    void close() override {
        std::cout << "[MockScanOp] Closed." << std::endl;
    }

private:
    std::vector<Tuple> mockData; 
    size_t currentIndex;         
};

#endif 