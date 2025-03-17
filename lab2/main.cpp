#include "BTree.h"
#include <iostream>

int main()
{
    BufferManager bm(20);
    BTree<int> tree("tree.bin", &bm);
    for (int i = 0; i < 100000; ++i) {
        tree.insert(i, {0, i});
    }
    for (int i = 0; i < 100000; ++i) {
        tree.insert(i, {0, i});
    }
    auto r1 = tree.search(10543);
    std::cout << "search(283): \n";
    for (auto r : r1) {
        std::cout << r.first << ", " << r.second << "\n";
    }
    auto r2 = tree.rangeSearch(50, 80);
    std::cout << "rangeSearch(50, 80): \n";
    for (auto r : r2) {
        std::cout << r.first << ", " << r.second << "\n";
    }
}