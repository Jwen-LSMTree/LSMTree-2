//
// Created by 김 정우 on 10/28/23.
//

#ifndef LSMTREE_SSTABLE2_H
#define LSMTREE_SSTABLE2_H

#include "../../bloom_filter/BloomFilter.h"
#include "../../sequence_number_filter/SequenceNumberFilter.h"
#include "DataBlockLocation.h"
#include "SSTableDataLocation.h"
#include "BlockLocation.h"
#include "SSTableId.h"
#include "../SearchResult.h"
#include "../../memory/SkipList.h"
#include "../../Entry.h"
#include "../../Option.h"
#include "../../Util.h"

#include <string>
#include <vector>
#include <cstdint>
#include <cstddef>

using namespace std;

class SSTable2{
public:
    explicit SSTable2(SSTableId id);

    explicit SSTable2(const SkipList &mem, SSTableId id);

    explicit SSTable2(const std::vector<Entry> &entries, size_t &pos, const SSTableId &id);

    SearchResult search(uint64_t key, uint64_t seqNum) const;

    SearchResult filterBySeqNum(uint64_t target_key, uint64_t target_seqNum, BlockLocation loc, uint64_t pos) const;

    vector<Entry> load() const;

    void remove() const;

    uint64_t getId() const;

    uint64_t getMinKey() const;

    uint64_t getMaxKey() const;

    uint64_t getMinSeqNum() const;

    uint64_t space() const;

    void print(uint64_t id) const;

    void setSeqNumFilter(const vector<uint64_t>& seqNums);

    SequenceNumberFilter seqNumFilter;

private:
    SSTableId id;
    uint64_t entryCnt;
    uint64_t blockCnt;
    uint64_t minKey;
    uint64_t maxKey;
    uint64_t minSeqNum;
    uint64_t indexBlockAddress;

    void save(vector<vector<uint64_t>> keys,
              vector<vector<uint64_t>> seqNums,
              vector<vector<uint64_t>> valueSizes,
              vector<vector<string>> values,
              vector<uint64_t> minKeys,
              vector<uint64_t> dataBlockOffsets);

    string loadValue(BlockLocation loc, uint64_t pos) const;

    uint64_t indexSpace() const;

    uint64_t blockSpace() const;

    string loadBlock(vector<uint64_t> dataBlockOffsets, uint64_t pos) const;

    BloomFilter bloomfilter;

    DataBlockLocation loadDataBlockLocation() const;

    BlockLocation loadBlockLocation(uint64_t pos) const;

    void setMinSeqNum(const vector<vector<uint64_t>> seqNums);

};

#endif //LSMTREE_SSTABLE2_H

