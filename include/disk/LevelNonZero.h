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
    explicit LevelNonZero(const string &dir);

    SearchResult search(uint64_t key, uint64_t seqNum) const;

    vector<Entry> flush();

    void merge(vector<Entry> &&entries1, uint64_t &no);

    void clear();

    uint64_t space() const;

    void print(uint64_t i) const;

private:
    string dir;
    uint64_t size;
    uint64_t byteCnt;
    list<SSTable> ssts;

    void save() const;
};

#endif