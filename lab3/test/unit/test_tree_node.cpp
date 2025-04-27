#include "Constants.h"
#include "TreeNode.h"
#include <cassert>
#include <cstring>
#include <iostream>

using namespace std;

int main()
{
    // Allocate a dummy page buffer of size MAX_PAGE_SIZE.
    uint8_t buffer[MAX_PAGE_SIZE];
    // Clear the buffer.
    memset(buffer, 0, MAX_PAGE_SIZE);

    // Initialize header fields:
    // - isLeaf: boolean (1 byte)
    // - size: int (4 bytes)
    // - next: int (4 bytes)
    bool initIsLeaf = false;
    memcpy(buffer, &initIsLeaf, sizeof(bool));
    int initSize = 0;
    memcpy(buffer + sizeof(bool), &initSize, sizeof(int));
    int initNext = -1;
    memcpy(buffer + sizeof(bool) + sizeof(int), &initNext, sizeof(int));

    // Create a TreeNode instance using int as key type.
    TreeNode<int> node(buffer);

    // Test setting and getting the leaf flag.
    node.setIsLeaf(true);
    assert(node.isLeaf() == true);
    cout << "isLeaf test passed." << endl;

    // Test setting and getting the size.
    node.setSize(3);
    assert(node.getSize() == 3);
    cout << "Size test passed." << endl;

    // Test setting and getting the next pointer.
    node.setNext(42);
    assert(node.getNext() == 42);
    cout << "Next pointer test passed." << endl;

    // Reset size to 0 for key–value insertion tests.
    node.setSize(0);

    // Test inserting key-value pairs.
    // We'll use int for both key and value.
    // Insert (10, 100) at position 0.
    node.template insertKeyValue<int>(10, 100, 0);
    node.setSize(node.getSize() + 1);
    // Insert (20, 200) at position 1.
    node.template insertKeyValue<int>(20, 200, 1);
    node.setSize(node.getSize() + 1);
    // Insert (30, 300) at position 2.
    node.template insertKeyValue<int>(30, 300, 2);
    node.setSize(node.getSize() + 1);

    // Test retrieving keys and values.
    int key0 = node.template getKey<int>(0);
    int value0 = node.template getValue<int>(0);
    int key1 = node.template getKey<int>(1);
    int value1 = node.template getValue<int>(1);
    int key2 = node.template getKey<int>(2);
    int value2 = node.template getValue<int>(2);

    assert(key0 == 10 && value0 == 100);
    assert(key1 == 20 && value1 == 200);
    assert(key2 == 30 && value2 == 300);
    cout << "Insertion and retrieval test passed." << endl;

    // Optionally, print the capacity of the node for int values.
    int cap = node.template capacity<int>();
    cout << "Calculated capacity: " << cap << " entries." << endl;

    cout << "All TreeNode tests passed." << endl;
    return 0;
}
