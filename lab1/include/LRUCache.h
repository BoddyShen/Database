#ifndef LRUCACHE_H
#define LRUCACHE_H

#include <unordered_map>
using namespace std;

struct Node {
    int val;
    Node *prev;
    Node *next;
    Node(int val);
};

class LRUCache
{
  public:
    LRUCache(int capacity);
    ~LRUCache();

    void put(int val);
    Node *getFirstNode();

  private:
    int _capacity;
    Node *head;
    Node *tail;
    unordered_map<int, Node *> cache;

    // Helper methods
    void removeNode(Node *node);
    void addToTail(Node *node);
    void moveToTail(Node *node);
};

#endif // LRUCACHE_H