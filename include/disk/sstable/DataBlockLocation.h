//
// Created by 김 정우 on 10/28/23.
//


#ifndef LSMTREE_DATABLOCKLOCATION_H
#define LSMTREE_DATABLOCKLOCATION_H

#include <cstdint>
#include <vector>

using namespace std;

class DataBlockLocation {
public:
    vector<uint64_t> minKeys;
    vector<uint64_t> dataBlockOffsets;

    DataBlockLocation(vector<uint64_t> minKeys, vector<uint64_t> dataBlockOffsets);

    DataBlockLocation() = default;
};

#endif //LSMTREE_DATABLOCKLOCATION_H
