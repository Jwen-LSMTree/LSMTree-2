#ifndef SSTABLE_DATA_LOCATION_H
#define SSTABLE_DATA_LOCATION_H

#include <cstdint>
#include <vector>

using namespace std;

class SSTableDataLocation {
public:
    vector<uint64_t> keys;
    vector<uint64_t> offsets;
    vector<uint64_t> seqNums;
    vector<uint64_t> dataBlockOffsets;

    SSTableDataLocation(vector<uint64_t> keys, vector<uint64_t> offsets, vector<uint64_t> seqNums,
                        vector<uint64_t> dataBlockOffsets);

    SSTableDataLocation() = default;
};

#endif