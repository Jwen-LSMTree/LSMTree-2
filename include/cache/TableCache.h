#ifndef TABLE_CACHE_H
#define TABLE_CACHE_H

#include "../disk/sstable/SSTableId.h"
#include "../Option.h"

#include <fstream>
#include <list>
#include <unordered_map>
#include <utility>
#include <cstdint>

using namespace std;

class TableCache {
public:
    ~TableCache();

    ifstream *open(SSTableId id);

    void close(SSTableId id);

private:
    list<pair<uint64_t, ifstream *>> linkedList;
    unordered_map<uint64_t, list<pair<uint64_t, ifstream *>>::iterator> hashMap;
};

#endif
