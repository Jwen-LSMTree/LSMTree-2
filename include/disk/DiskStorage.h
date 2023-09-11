#ifndef DISK_STORAGE_H
#define DISK_STORAGE_H

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

    void flush(const SkipList &mem);

    SearchResult search(uint64_t key, uint64_t seqNum);

    void clear();

    LevelZero *getLevelZero();

    void print() const;

private:
    string dir;
    uint64_t id;

    LevelZero levelZero;
    vector<LevelNonZero> levelNonZeros;

    void save() const;
};

#endif