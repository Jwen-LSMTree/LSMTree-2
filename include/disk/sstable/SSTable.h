#ifndef SSTABLE_H
#define SSTABLE_H

#include "../../bloom_filter/BloomFilter.h"
#include "SSTableDataLocation.h"
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

class SSTable {
public:
    explicit SSTable(SSTableId id);

    explicit SSTable(const SkipList &mem, SSTableId id);

    explicit SSTable(const std::vector<Entry> &entries, size_t &pos, const SSTableId &id);

    SearchResult search(uint64_t key, uint64_t seqNum) const;

    SearchResult filterBySeqNum(uint64_t target_key, uint64_t target_seqNum, SSTableDataLocation loc, uint64_t pos) const;

    vector<Entry> load() const;

    void remove() const;

    uint64_t getId() const;

    uint64_t getMinKey() const;

    uint64_t getMaxKey() const;

    uint64_t space() const;

    void print(uint64_t id) const;

private:
    SSTableId id;
    uint64_t entryCnt;
    uint64_t blockCnt;
    uint64_t minKey;
    uint64_t maxKey;
    uint64_t size;

    void save(vector<uint64_t> keys, vector<uint64_t> offsets, vector<uint64_t> seqNums,
              vector<uint64_t> dataBlockOffsets, const string &blockSegment);

    string loadValue(SSTableDataLocation loc, uint64_t pos) const;

    uint64_t indexSpace() const;

    uint64_t blockSpace() const;

    SSTableDataLocation loadAll() const;

    string loadBlock(vector<uint64_t> dataBlockOffsets, uint64_t pos) const;

    BloomFilter bloomfilter;
};

#endif
