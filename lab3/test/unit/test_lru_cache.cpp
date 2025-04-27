#include "LRUCache.h"
#include <cassert>
#include <iostream>

void test_put() {
    cout << "Test: LRUCache put" << endl;
    LRUCache cache(3);
    cache.put(1);
    cache.put(2);
    cache.put(3);

    assert(cache.getSize() == 3);
    
    Node* firstNode = cache.getFirstNode();
    assert(firstNode->val == 1);
    assert(firstNode->next->val == 2);
    assert(firstNode->next->next->val == 3);
    
    std::cout << "Test: LRUCache put func passed!" << std::endl;
}

void testLRUBehavior() {
    std::cout << "Testing LRU behavior" << std::endl;
    LRUCache cache(3);
    
    cache.put(1);
    cache.put(2);
    cache.put(3);
    
    cache.put(1);
    
    Node* firstNode = cache.getFirstNode();
    assert(firstNode->val == 2);
    assert(firstNode->next->val == 3);
    assert(firstNode->next->next->val == 1);

    cache.put(2);
    firstNode = cache.getFirstNode();
    assert(firstNode->val == 3);
    assert(firstNode->next->val == 1);
    assert(firstNode->next->next->val == 2);
    
    std::cout << "LRU behavior passed" << std::endl;
}

void test_remove() {
    std::cout << "Test: remove()" << std::endl;
    LRUCache cache(3);
    
    cache.put(1);
    cache.put(2);
    cache.put(3);
    cache.remove(2);
    
    assert(cache.getSize() == 2);
    
    Node* firstNode = cache.getFirstNode();
    assert(firstNode->val == 1);
    assert(firstNode->next->val == 3);
    
    std::cout << "remove() passed!" << std::endl;
}

// Test updating existing elements
void test_update_existing() {
    std::cout << "Testing update of existing elements..." << std::endl;
    LRUCache cache(3);
    
    // Add initial elements
    cache.put(1);
    cache.put(2);
    cache.put(3);
    
    // Update element multiple times
    cache.put(2);
    cache.put(2);
    cache.put(2);
    
    assert(cache.getSize() == 3);  // Size shouldn't change
    
    Node* firstNode = cache.getFirstNode();
    assert(firstNode->val == 1);
    assert(firstNode->next->val == 3);
    assert(firstNode->next->next->val == 2);  // 2 should be most recently used
    
    std::cout << "Update existing elements test passed!" << std::endl;
}

// Test empty cache operations
void test_empty_cache() {
    std::cout << "Testing empty cache operations..." << std::endl;
    LRUCache cache(3);
    
    assert(cache.getSize() == 0);  // Initial size should be 0
    
    // Add and remove to get back to empty
    cache.put(1);
    assert(cache.getSize() == 1);
    cache.remove(1);
    assert(cache.getSize() == 0);
    
    // Add multiple and remove all
    cache.put(1);
    cache.put(2);
    cache.remove(1);
    cache.remove(2);
    assert(cache.getSize() == 0);
    
    std::cout << "Empty cache operations test passed!" << std::endl;
}

int main() {
    std::cout << "### Running LRUCache Tests ###" << std::endl;
    
    test_put();
    testLRUBehavior();
    test_remove();
    test_update_existing();
    test_empty_cache();
    
    std::cout << "All tests passed!" << std::endl;
    return 0;
}