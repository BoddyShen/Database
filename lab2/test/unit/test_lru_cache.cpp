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


int main() {
    std::cout << "### Running LRUCache Tests ###" << std::endl;
    
    test_put();
    testLRUBehavior();
    test_remove();
    
    std::cout << "tests passed" << std::endl;
    return 0;
}