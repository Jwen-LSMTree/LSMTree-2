#ifndef SSTABLE_DATA_LOCATION_H
#define SSTABLE_DATA_LOCATION_H

#include <vector>

using namespace std;

class SSTableDataLocation {
public:
    vector<uint64_t> keys;
    vector<uint64_t> offsets;
    vector<uint64_t> seqNums;
    vector<uint64_t> oris;

    SSTableDataLocation(vector<uint64_t> keys, vector<uint64_t> offsets, vector<uint64_t> seqNums,
                        vector<uint64_t> oris);

    SSTableDataLocation() = default;
};

#endif