#ifndef DISK_STORAGE_H
#define DISK_STORAGE_H

#include "../cache/TableCache.h"
#include "LevelZero.h"
#include "LevelNonZero.h"
#include "SearchResult.h"
#include "../memory/SkipList.h"
#include "../Option.h"
#include "../Util.h"

#include <string>
#include <cstdint>
#include <vector>

using namespace std;

class DiskStorage {
public:
    explicit DiskStorage(const string &dir);

    void add(const SkipList &mem);

    SearchResult search(uint64_t key, uint64_t seqNum);

    void clear();

    LevelZero *getLevelZero();

private:
    string dir;
    uint64_t no;

    LevelZero level0;
    vector<LevelNonZero> levels;

    TableCache tableCache;

    void save() const;
};

#endif