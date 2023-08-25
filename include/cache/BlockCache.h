#ifndef CACHE_H
#define CACHE_H

#include "../disk/sstable/SSTable.h"
#include "../disk/Location.h"
#include "../disk/SearchResult.h"
#include "../Option.h"

#include <string>
#include <list>
#include <utility>
#include <unordered_map>
#include <cstdint>

using namespace std;

class BlockCache {
public:
    explicit BlockCache();

    void complete(SearchResult &result);

private:
    uint64_t byteCnt;
    list<pair<string, string>> linkedList;
    unordered_map<string, list<pair<string, string>>::iterator> hashMap;

    string read(Location location);
};

#endif
