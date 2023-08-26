#ifndef LEVEL_ZERO_H
#define LEVEL_ZERO_H

#include "../cache/TableCache.h"
#include "sstable/SSTable.h"
#include "sstable/SSTableId.h"
#include "SearchResult.h"
#include "../memory/SkipList.h"
#include "../Entry.h"
#include "../Util.h"

#include <string>
#include <cstdint>
#include <vector>

using namespace std;

class LevelZero {
public:
    explicit LevelZero(const string &dir, TableCache *tableCache);

    SearchResult search(uint64_t key, uint64_t seqNum) const;

    void add(const SkipList &mem, uint64_t &no);

    vector<Entry> extract();

    void clear();

    uint64_t space() const;

private:
    string dir;
    uint64_t size;
    uint64_t byteCnt;
    vector<SSTable> ssts;
    TableCache *tableCache;

    void save() const;
};

#endif