#ifndef BNLJOINOP_H
#define BNLJOINOP_H

#include "BufferManager.h"
#include "Operator.h"
#include "functional"

template <typename KeyType, typename LeftRowType> class BNLJoinOp : public Operator
{
  public:
    BNLJoinOp(BufferManager &bm, Operator *left, Operator *right, int blockSize,
              std::string tempFileName,

              std::function<KeyType(const Tuple &)> leftKeyExtractor,
              std::function<KeyType(const Tuple &)> rightKeyExtractor,
              std::unordered_map<std::string, int> const &idx_map)
        : bm(bm), left(left), right(right), blockFileName(tempFileName), blockSize(blockSize),
          extractLeftKey(std::move(leftKeyExtractor)),
          extractRightKey(std::move(rightKeyExtractor)), idx_map(idx_map)
    {
        if (blockSize < 1) {
            std::cerr << "Error: block size must be greater than 0\n";
            exit(1);
        }
        // remove the temp file if it exists
        remove(blockFileName.c_str());
        bm.registerFile(blockFileName);
    }

    void open() override
    {
        left->open();
        right->open();
        blockPages.clear();
        blockHashTable.clear();
    }

    bool next(Tuple &out) override
    {
        // Build the block hash table
        if (!buildBlockDone) {
            buildNextBlock();
            if (blockPages.empty()) // <— no pages means left side was empty or fully scanned
                return false;
            buildBlockDone = true;
        }

        // blockHashTable is used to store the hash table for the current block
        // key -> (pid, slotId)
        // The lastRightTuple is used to store the last tuple from the right operator
        // In the blockHashTable, probeList is used to store the list of tuples that match the key
        // Each can make a join with the lastRightTuple
        if (probeIdx < probeList.size()) {
            auto [pid, slotId] = probeList[probeIdx++];
            out = makeJoinedTuple(pid, slotId, lastRightTuple);
            return true;
        }

        // else if probeList is used up, we move to next right tuple and corresponding inner probe
        Tuple rightT;
        int counter = 0;
        while (right->next(rightT)) {
            counter++;
            lastRightTuple = rightT;
            auto key = extractRightKey(lastRightTuple);
            auto it = blockHashTable.find(key);
            if (it != blockHashTable.end()) {
                probeList = it->second;
                probeIdx = 1; // we’ll consume the 0th entry immediately
                auto [pid, slotId] = probeList[0];
                out = makeJoinedTuple(pid, slotId, rightT);
                return true;
            }
        }
        cout << "Use " << counter << " right tuples in join" << endl;

        // if no more tuples, we need to build the next block and rescan the right operator
        right->close();
        right->open();
        // clear previous block
        for (int pid : blockPages) bm.unpinPage(pid, blockFileName);
        blockPages.clear();
        blockHashTable.clear();
        buildBlockDone = false;
        return next(out);
    }

    void close() override
    {
        left->close();
        right->close();
        // for (int pid : blockPages) bm.unpinPage(pid, blockFileName);
        blockPages.clear();
        blockHashTable.clear();
        probeList.clear();
        probeIdx = 0;
        remove(blockFileName.c_str());
    }

    int getTotalOut() { return total_out_tuple; }

  private:
    BufferManager &bm;
    Operator *left;
    Operator *right;

    // workspace for probing
    Tuple lastRightTuple;
    std::vector<std::pair<int, int>> probeList;
    size_t probeIdx = 0;

    // block
    std::string blockFileName;
    std::size_t blockSize;
    std::vector<int> blockPages{}; // pids of the blocks
    std::unordered_map<KeyType, std::vector<std::pair<int, int>>>
        blockHashTable{}; // key -> (pid, rowId)
    bool buildBlockDone = false;

    std::function<KeyType(const Tuple &)> extractLeftKey;
    std::function<KeyType(const Tuple &)> extractRightKey;
    unordered_map<std::string, int> const idx_map; // for mapping row fields to indices of the tuple
    int total_left_tuple = 0;
    int total_out_tuple = 0;

    // helper function
    // build the next block from left operator
    void buildNextBlock()
    {
        if (blockPages.size() > 0) {
            for (int pid : blockPages) {
                bm.unpinPage(pid, blockFileName);
            }
        }

        // clear previous block
        blockPages.clear();
        blockHashTable.clear();

        // keep inserting until the block is full
        Page<LeftRowType> *currentPage = nullptr;
        Tuple in;
        while (blockPages.size() < blockSize && left->next(in)) {
            total_left_tuple++;
            // create a new page if needed
            if (!currentPage || currentPage->isFull()) {
                currentPage = bm.createPage<LeftRowType>(blockFileName);
                blockPages.push_back(currentPage->getPid());
            }

            // insert the row into the page
            if (in.fields.size() < idx_map.size()) {
                cerr << "Error: not enough fields in the tuple\n";
                continue;
            }

            LeftRowType row = LeftRowType(in, idx_map);
            int slotId = currentPage->insertRow(row);

            // extract key from row and add into the hash table
            KeyType key = extractLeftKey(in);
            blockHashTable[key].emplace_back(currentPage->getPid(), slotId);
        }
        cout << "Current " << total_left_tuple << " used left tuples" << endl;
    }

    Tuple makeJoinedTuple(int pid, int slotId, const Tuple &lastRightTuple)
    {
        Page<LeftRowType> *page = bm.getPage<LeftRowType>(pid, blockFileName);
        LeftRowType *leftRow = page->getRow(slotId);
        bm.unpinPage(pid, blockFileName);

        Tuple result = leftRow->toTuple();
        delete leftRow;
        result.fields.insert(result.fields.end(), lastRightTuple.fields.begin(),
                             lastRightTuple.fields.end());
        total_out_tuple++;
        return result;
    }
};
#endif // BNLJOINOP_H