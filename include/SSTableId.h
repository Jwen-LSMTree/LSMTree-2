#ifndef SSTABLE_ID_H
#define SSTABLE_ID_H

#include <string>
#include <cstdint>

using namespace std;

struct SSTableId {
    string dir;
    uint64_t no;

    SSTableId() = default;

    SSTableId(const string &dir, uint64_t no);

    string name() const;
};

#endif