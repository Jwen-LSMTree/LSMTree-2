#ifndef ENTRY_H
#define ENTRY_H

#include <cstdint>
#include <string>

using namespace std;

struct Entry {
    uint64_t key;
    string value;
    uint64_t seqNum = 0;

    Entry(uint64_t key, string value);

    Entry(uint64_t key, string value, uint64_t seqNum);
};

#endif
