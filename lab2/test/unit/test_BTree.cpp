#include "BTree.h"
#include "DatabaseCatalog.h"
#include <cassert>
#include <chrono>
#include <iostream>

using namespace std;

// Test cases for BTree operations
void test_bulk_insert()
{
    std::cout << "testing bulk insert, point search, range search\n";
    BufferManager bm(20);
    BTree<int> tree("tree.bin", &bm);
    std::vector<std::pair<int, Rid>> data;
    for (int i = 0; i < 100000; ++i) {
        data.push_back({i, {0, i}});
    }
    auto start = std::chrono::high_resolution_clock::now();
    tree.bulkInsert(data);
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "bulk insert time: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
              << "ms\n";
    auto r1 = tree.search(283);
    std::cout << "search(283): \n";
    assert(r1[0].second == 283);
    std::cout << "point search passed\n";
    auto r2 = tree.rangeSearch(50, 80);
    std::cout << "rangeSearch(50, 80): \n";
    for (int i = 50; i <= 80; ++i) {
        assert(r2[i - 50].second == i);
    }
    std::cout << "range search passed\n";
}

void test_insert()
{
    std::cout << "testing load index, insert, duplicate key\n";
    BufferManager bm(20);
    BTree<int> tree("tree.bin", &bm);
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 100000; ++i) {
        tree.insert(i, {0, i});
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "One by one insertion time: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
              << "ms\n";
    auto r1 = tree.search(283);
    std::cout << "search(283): \n";
    assert(r1[0].second == 283 && r1[1].second == 283);
    std::cout << "point search with duplicate key passed\n";
    auto r2 = tree.rangeSearch(50, 80);
    std::cout << "rangeSearch(50, 80): \n";
    for (int i = 50; i <= 80; ++i) {
        assert(r2[(i - 50) * 2].second == i && r2[(i - 50) * 2 + 1].second == i);
    }
    std::cout << "range search with duplicate key passed\n";
}

// Test small dataset operations
void test_small_dataset()
{
    std::cout << "testing small dataset operations\n";
    BufferManager bm(20);
    BTree<int> tree("small_tree.bin", &bm);
    
    // Insert a small number of entries
    for (int i = 0; i < 10; ++i) {
        tree.insert(i * 10, {0, i * 10});  // Insert values 0, 10, 20, ..., 90
    }
    
    // Test point search
    auto r1 = tree.search(30);
    assert(r1.size() == 1);
    assert(r1[0].first == 0 && r1[0].second == 30);
    std::cout << "Small dataset point search passed\n";
    
    // Test range search on small dataset
    auto r2 = tree.rangeSearch(25, 45);
    assert(r2.size() == 2);  // Should find 30 and 40
    assert(r2[0].second == 30);
    assert(r2[1].second == 40);
    std::cout << "Small dataset range search passed\n";
}

// Test sequential operations
void test_sequential_operations()
{
    std::cout << "testing sequential operations\n";
    BufferManager bm(20);
    BTree<int> tree("seq_tree.bin", &bm);
    
    // Test 1: Insert and immediately search
    tree.insert(100, {1, 100});
    auto r1 = tree.search(100);
    assert(r1.size() == 1);
    assert(r1[0].first == 1 && r1[0].second == 100);
    std::cout << "Immediate search after insert passed\n";
    
    // Test 2: Insert multiple values and search range
    tree.insert(200, {1, 200});
    tree.insert(300, {1, 300});
    tree.insert(400, {1, 400});
    
    auto r2 = tree.rangeSearch(150, 350);
    assert(r2.size() == 2);  // Should find 200 and 300
    assert(r2[0].second == 200);
    assert(r2[1].second == 300);
    std::cout << "Sequential insert and range search passed\n";
}

int main()
{
    test_bulk_insert();
    test_insert();
    test_small_dataset();
    test_sequential_operations();
    
    std::cout << "All tests passed!\n";
    return 0;
}