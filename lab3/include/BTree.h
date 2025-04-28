#ifndef B_TREE_H
#define B_TREE_H

#include "BufferManager.h"
#include "TreeNode.h"
#include <vector>

// Rid = <pid, sid>
using Rid = std::pair<int, int>;

template <typename K> class BTree
{
  public:
    /**
     * Constructor that initializes the BTree.
     * @param filePath The file path where the BTree is stored.
     * @param bm The buffer manager.
     */
    BTree(const std::string filePath, BufferManager *bm);

    /**
     * Destructor. Update the header page and flush all dirty pages.
     */
    ~BTree();

    /**
     * Inserts a key-value pair into the B+ tree.
     *
     * @param key The key to insert.
     * @param rid The value associated with the key.
     */
    void insert(K key, Rid r);

    /**
     * Bulk inserts a vector of key-value pairs into the B+ tree.
     *
     * @param data A vector of key-value pairs to insert.
     */
    void bulkInsert(std::vector<std::pair<K, Rid>> &data);

    /**
     * Searches for a value by key in the B+ tree.
     *
     * @param  key The key to search for.
     * @return A vector of Rids with the required value of the search attribute.
     */
    std::vector<Rid> search(K key);

    /**
     * Performs a range query, retrieving all key-value pairs in the specified range.
     *
     * @param  startKey The start of the range (inclusive).
     * @param  endKey   The end of the range (inclusive).
     * @return A vector of Rids with values of the search attribute that fall within the range.
     */
    std::vector<Rid> rangeSearch(K startKey, K endKey);

  private:
    // Helper function. Find the first leaf node that contains key
    int findLeaf(K key);

    void insertIntoParent(K key, int n1, int n2);

    std::string filePath;
    BufferManager *bm;
    // pid of root
    int root;
    std::unordered_map<int, int> parents;
};

template <typename K>
BTree<K>::BTree(const std::string filePath, BufferManager *bm)
    : filePath(filePath), bm(bm), root(-1)
{
    bool is_existed = bm->registerFile(filePath);
    if (is_existed) {
        // read the header page, <char> is dummy type
        Page<char> *header_page = bm->getPage<char>(0, filePath);
        memcpy(&root, header_page->getPageData(), sizeof(root));
        bm->unpinPage(0, filePath);
    } else {
        // reserve the first page for the header page
        bm->createPage<char>(filePath);
        bm->unpinPage(0, filePath);
    }
}

template <typename K> BTree<K>::~BTree()
{
    Page<char> *header_page = bm->getPage<char>(0, filePath);
    memcpy(header_page->getPageData(), &root, sizeof(root));
    bm->markDirty(0, filePath);
    bm->unpinPage(0, filePath);
    bm->force();
}

template <typename K> void BTree<K>::insert(K key, Rid r)
{
    // find the first leaf node that contains key
    int leaf_id = -1;
    if (root == -1) {
        // if tree is empty, create a leaf node as root node
        Page<char> *page = bm->createPage<char>(filePath);
        TreeNode<K> node(page->getPageData());
        node.setSize(0);
        node.setNext(-1);
        node.setIsLeaf(true);
        root = page->getPid();
        leaf_id = root;
        bm->unpinPage(root, filePath);
    } else {
        leaf_id = findLeaf(key);
    }
    Page<char> *leaf_page = bm->getPage<char>(leaf_id, filePath);
    TreeNode<K> leaf(leaf_page->getPageData());

    // insert key value pair into leaf
    int pos = leaf.getSize();
    for (int i = 0; i < leaf.getSize(); ++i) {
        if (key < leaf.template getKey<Rid>(i)) {
            pos = i;
            break;
        }
    }
    leaf.template insertKeyValue<Rid>(key, r, pos);
    leaf.setSize(leaf.getSize() + 1);

    bm->markDirty(leaf_id, filePath);
    bm->unpinPage(leaf_id, filePath);

    // if leaf is full, split leaf
    if (leaf.getSize() > leaf.template capacity<Rid>()) {
        Page<char> *new_page = bm->createPage<char>(filePath);
        TreeNode<K> new_leaf(new_page->getPageData());
        new_leaf.setIsLeaf(true);
        new_leaf.setNext(leaf.getNext());
        leaf.setNext(new_page->getPid());
        // copy the key value pairs starting at ceil(n / 2) to new leaf
        int split_line = leaf.template capacity<Rid>() / 2 + leaf.template capacity<Rid>() % 2;
        for (int i = split_line; i < leaf.getSize(); ++i) {
            new_leaf.template insertKeyValue<Rid>(leaf.template getKey<Rid>(i),
                                                  leaf.template getValue<Rid>(i), i - split_line);
        }
        new_leaf.setSize(leaf.getSize() - split_line);
        leaf.setSize(split_line);

        // insert smallest key of new leaf into parent
        K k = new_leaf.template getKey<Rid>(0);

        bm->unpinPage(new_page->getPid(), filePath);

        insertIntoParent(k, leaf_page->getPid(), new_page->getPid());
    }
}

template <typename K> void BTree<K>::insertIntoParent(K key, int n1, int n2)
{
    // if n1 is the root, create a new node as root and set it as parent
    if (n1 == root) {
        Page<char> *page = bm->createPage<char>(filePath);
        root = page->getPid();
        TreeNode<K> parent(page->getPageData());
        parent.setIsLeaf(false);
        parent.template insertValue<int>(n1, 0);
        parents[n1] = root;
        parent.setSize(1);
        bm->unpinPage(root, filePath);
    }

    // insert (key, n2) into parent
    TreeNode<K> parent(bm->getPage<char>(parents[n1], filePath)->getPageData());
    for (int i = 0; i < parent.getSize(); ++i) {
        if (n1 == parent.template getValue<int>(i)) {
            parent.template insertKeyValue<int>(key, n2, i + 1);
            break;
        }
    }
    parent.setSize(parent.getSize() + 1);

    bm->markDirty(parents[n1], filePath);
    bm->unpinPage(parents[n1], filePath);

    // if parent is full, split parent
    if (parent.getSize() > parent.template capacity<int>()) {
        Page<char> *new_page = bm->createPage<char>(filePath);
        TreeNode<K> new_node(new_page->getPageData());
        new_node.setIsLeaf(false);
        // copy the key value pairs starting at ceil(n / 2) to new node
        int split_line = parent.template capacity<int>() / 2 + parent.template capacity<int>() % 2;
        new_node.template insertValue<int>(parent.template getValue<int>(split_line), 0);
        for (int i = split_line + 1; i < parent.getSize(); ++i) {
            new_node.template insertKeyValue<int>(parent.template getKey<int>(i),
                                                  parent.template getValue<int>(i), i - split_line);
        }
        K k = parent.template getKey<int>(split_line);
        new_node.setSize(parent.getSize() - split_line);
        parent.setSize(split_line);

        bm->unpinPage(new_page->getPid(), filePath);

        insertIntoParent(k, parents[n1], new_page->getPid());
    }
}

template <typename K> void BTree<K>::bulkInsert(std::vector<std::pair<K, Rid>> &data)
{
    // the tree should be empty when bulk insert
    if (root != -1) {
        std::cerr << "The tree should be empty when bulk insert\n";
        return;
    }

    Page<char> *page = bm->createPage<char>(filePath);
    TreeNode<K> leaf(page->getPageData());
    leaf.setIsLeaf(true);
    leaf.setSize(0);
    leaf.setNext(-1);
    int leaf_id = page->getPid();

    Page<char> *root_page = bm->createPage<char>(filePath);
    TreeNode<K> root_node(root_page->getPageData());
    root_node.setIsLeaf(false);
    root = root_page->getPid();
    root_node.template insertValue<int>(leaf_id, 0);
    root_node.setSize(1);
    parents[leaf_id] = root;
    bm->unpinPage(root, filePath);

    for (auto &record : data) {
        // if current leaf is full, create a new leaf
        if (leaf.getSize() == leaf.template capacity<Rid>()) {
            Page<char> *new_page = bm->createPage<char>(filePath);
            TreeNode<K> new_leaf(new_page->getPageData());
            int new_leaf_id = new_page->getPid();
            new_leaf.setIsLeaf(true);
            leaf.setNext(new_leaf_id);
            new_leaf.template insertKeyValue<Rid>(record.first, record.second, 0);
            new_leaf.setSize(1);
            insertIntoParent(record.first, leaf_id, new_leaf_id);
            bm->unpinPage(leaf_id, filePath);
            leaf_id = new_leaf_id;
            leaf = new_leaf;
        } else {
            leaf.template insertKeyValue<Rid>(record.first, record.second, leaf.getSize());
            leaf.setSize(leaf.getSize() + 1);
        }
    }
    bm->unpinPage(leaf_id, filePath);
}

template <typename K> std::vector<Rid> BTree<K>::search(K key)
{
    std::vector<Rid> results;
    int leaf = findLeaf(key);
    TreeNode<K> node(bm->getPage<char>(leaf, filePath)->getPageData());
    int pos = 0;
    while (node.template getKey<Rid>(pos) <= key) {
        if (node.template getKey<Rid>(pos) == key) {
            results.push_back(node.template getValue<Rid>(pos));
        }
        pos++;
        if (pos == node.getSize()) {
            bm->unpinPage(leaf, filePath);
            leaf = node.getNext();
            node = TreeNode<K>(bm->getPage<char>(leaf, filePath)->getPageData());
            pos = 0;
        }
    }
    bm->unpinPage(leaf, filePath);
    return results;
}

template <typename K> std::vector<Rid> BTree<K>::rangeSearch(K startKey, K endKey)
{
    std::vector<Rid> results;
    int leaf = findLeaf(startKey);
    TreeNode<K> node(bm->getPage<char>(leaf, filePath)->getPageData());
    int pos = 0;
    while (node.template getKey<Rid>(pos) <= endKey) {
        if (node.template getKey<Rid>(pos) >= startKey &&
            node.template getKey<Rid>(pos) <= endKey) {
            results.push_back(node.template getValue<Rid>(pos));
        }
        pos++;
        if (pos == node.getSize()) {
            bm->unpinPage(leaf, filePath);
            leaf = node.getNext();
            node = TreeNode<K>(bm->getPage<char>(leaf, filePath)->getPageData());
            pos = 0;
        }
    }
    bm->unpinPage(leaf, filePath);
    return results;
}

template <typename K> int BTree<K>::findLeaf(K key)
{
    parents = std::unordered_map<int, int>();
    int cur_pid = root;
    TreeNode<K> cur_node(bm->getPage<char>(cur_pid, filePath)->getPageData());
    while (!cur_node.isLeaf()) {
        int pos = cur_node.getSize() - 1;
        for (int i = 1; i < cur_node.getSize(); ++i) {
            if (key <= cur_node.template getKey<int>(i)) {
                pos = i - 1;
                break;
            }
        }
        int child = cur_node.template getValue<int>(pos);
        parents[child] = cur_pid;
        bm->unpinPage(cur_pid, filePath);
        cur_pid = child;
        cur_node = TreeNode<K>(bm->getPage<char>(cur_pid, filePath)->getPageData());
    }
    bm->unpinPage(cur_pid, filePath);
    return cur_pid;
}

#endif // B_TREE_H