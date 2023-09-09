#ifndef SSTABLE_H
#define SSTABLE_H

#include "../../bloom_filter/BloomFilter.h"
#include "../../sequence_number_filter/SequenceNumberFilter.h"
#include "SSTableDataLocation.h"
#include "SSTableId.h"
#include "../Location.h"
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

    uint64_t number() const;

    uint64_t lower() const;

    uint64_t upper() const;

    uint64_t space() const;

    void print(uint64_t id) const;

    void setSeqNumFilter(const vector<uint64_t>& seqNums);

private:
    SSTableId id;
    uint64_t entryCnt;

    uint64_t blockCnt;
    uint64_t min;
    uint64_t max;
    uint64_t size;

    void save(vector<uint64_t> keys, vector<uint64_t> offsets, vector<uint64_t> seqNums,
              vector<uint64_t> oris, vector<uint64_t> cmps, const string &blockSeg);

    Location locate(SSTableDataLocation loc, uint64_t pos) const;

    uint64_t indexSpace() const;

    uint64_t blockSpace() const;

    SSTableDataLocation loadAll() const;

    string loadBlock(vector<uint64_t> cmps, uint64_t pos) const;

    BloomFilter bloomfilter;

    SequenceNumberFilter seqNumFilter;
};

#endif
