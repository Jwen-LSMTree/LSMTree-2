#ifndef LEVEL_NON_ZERO_H
#define LEVEL_NON_ZERO_H

#include "sstable/SSTable.h"
#include "SearchResult.h"
#include "../Util.h"

#include <string>
#include <cstdint>
#include <vector>
#include <list>

using namespace std;

class LevelNonZero {
public:
    explicit LevelNonZero(const string &dir, TableCache *tableCache);

    SearchResult search(uint64_t key) const;

    vector<Entry> extract();

    void merge(vector<Entry> &&entries1, uint64_t &no);

    void clear();

    uint64_t space() const;

private:
    string dir;
    uint64_t size;
    uint64_t byteCnt;
    uint64_t lastKey;
    list<SSTable> ssts;
    TableCache *tableCache;

    void save() const;
};

#endif