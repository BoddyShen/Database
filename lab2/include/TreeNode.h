#ifndef TREE_NODE_H
#define TREE_NODE_H

#include "Constants.h"
#include <cassert>
#include <iostream>

// internal node: V = pid; leaf node: V = Rid
template <typename K> class TreeNode
{
  public:
    TreeNode(uint8_t *page_data);

    bool isLeaf() const;

    void setIsLeaf(bool isLeaf);

    template <typename V> int capacity() const;

    int getSize() const;

    void setSize(int size);

    template <typename V> K getKey(int i) const;

    template <typename V> V getValue(int i) const;

    template <typename V> void insertValue(V v, int i);

    template <typename V> void insertKeyValue(K k, V v, int i);

    int getNext() const;

    void setNext(int pid);

  private:
    uint8_t *data;
    uint8_t *storage;
};

template <typename K> TreeNode<K>::TreeNode(uint8_t *page_data) : data(page_data)
{
    storage = data + sizeof(bool) + 2 * sizeof(int);
}

template <typename K> bool TreeNode<K>::isLeaf() const { return *((bool *)data); }

template <typename K> void TreeNode<K>::setIsLeaf(bool isLeaf)
{
    memcpy(data, &isLeaf, sizeof(bool));
}

template <typename K> template <typename V> int TreeNode<K>::capacity() const
{
    return (MAX_PAGE_SIZE - sizeof(bool) - 2 * sizeof(int)) / (sizeof(K) + sizeof(V)) - 1;
}

template <typename K> int TreeNode<K>::getSize() const { return *(int *)(data + sizeof(bool)); }

template <typename K> void TreeNode<K>::setSize(int size)
{
    memcpy(data + sizeof(bool), &size, sizeof(int));
}

template <typename K> template <typename V> K TreeNode<K>::getKey(int i) const
{
    return *(K *)(storage + i * (sizeof(K) + sizeof(V)));
}

template <typename K> template <typename V> V TreeNode<K>::getValue(int i) const
{
    return *(V *)(storage + i * (sizeof(K) + sizeof(V)) + sizeof(K));
}

template <typename K> template <typename V> void TreeNode<K>::insertValue(V v, int i)
{
    memcpy(storage + i * (sizeof(K) + sizeof(V)) + sizeof(K), &v, sizeof(V));
}

template <typename K> template <typename V> void TreeNode<K>::insertKeyValue(K k, V v, int i)
{
    auto pos = storage + i * (sizeof(K) + sizeof(V));
    if (i < getSize()) {
        memcpy(pos + sizeof(K) + sizeof(V), pos, (getSize() - i) * (sizeof(K) + sizeof(V)));
    }
    memcpy(pos, &k, sizeof(K));
    memcpy(pos + sizeof(K), &v, sizeof(V));
}

template <typename K> int TreeNode<K>::getNext() const
{
    return *(int *)(data + sizeof(bool) + sizeof(int));
}

template <typename K> void TreeNode<K>::setNext(int pid)
{
    memcpy(data + sizeof(bool) + sizeof(int), &pid, sizeof(int));
}

#endif // TREE_NODE_H
