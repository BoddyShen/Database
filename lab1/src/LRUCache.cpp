#include "LRUCache.h"

Node::Node(int val) : val(val), prev(nullptr), next(nullptr) {}

// LRUCache constructor: Initialize dummy head and tail.
LRUCache::LRUCache(int capacity) : _capacity(capacity)
{
    head = new Node(0);
    tail = new Node(0);
    head->next = tail;
    tail->prev = head;
}

// Destructor to free all allocated nodes.
LRUCache::~LRUCache()
{
    Node *curr = head;
    while (curr) {
        Node *next = curr->next;
        delete curr;
        curr = next;
    }
}

// Put val into the cache, update position in LRU.
void LRUCache::put(int val)
{
    if (cache.find(val) != cache.end()) {
        Node *node = cache[val];
        moveToTail(node);
    } else {
        if (cache.size() >= _capacity) {
            // Remove the least recently used node (first real node after head).
            Node *lru = head->next;
            removeNode(lru);
            cache.erase(lru->val);
            delete lru;
        }
        Node *newNode = new Node(val);
        cache[val] = newNode;
        addToTail(newNode);
    }
}

Node *LRUCache::getFirstNode() { return head->next; }

// Helper method: Remove a node from the doubly linked list.
void LRUCache::removeNode(Node *node)
{
    Node *prev_node = node->prev;
    Node *next_node = node->next;
    prev_node->next = next_node;
    next_node->prev = prev_node;
}

// Helper method: Add a node right before the tail.
void LRUCache::addToTail(Node *node)
{
    Node *prev_tail = tail->prev;
    prev_tail->next = node;
    node->prev = prev_tail;
    node->next = tail;
    tail->prev = node;
}

// Helper method: Move an existing node to the tail.
void LRUCache::moveToTail(Node *node)
{
    removeNode(node);
    addToTail(node);
}