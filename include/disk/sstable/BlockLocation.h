//
// Created by 김 정우 on 10/28/23.
//

#ifndef LSMTREE_BLOCKLOCATION_H
#define LSMTREE_BLOCKLOCATION_H


#include <cstdint>
#include <vector>

using namespace std;

class BlockLocation {
public:
    vector<uint64_t> keys;
    vector<uint64_t> seqNums;
    vector<uint64_t> valueSizes;

    BlockLocation(vector<uint64_t> keys, vector<uint64_t> seqNums, vector<uint64_t> valueSizes);

    BlockLocation() = default;
};

#endif //LSMTREE_BLOCKLOCATION_H
