#include "BTree.h"
#include <chrono>
#include <iostream>

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

int main()
{
    test_bulk_insert();
    test_insert();
}