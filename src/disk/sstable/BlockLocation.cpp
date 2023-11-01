//
// Created by 김 정우 on 10/28/23.
//

#include "../../../include/disk/sstable/BlockLocation.h"

BlockLocation::BlockLocation(vector<uint64_t> keys, vector<uint64_t> seqNums, vector<uint64_t> valueSizes)
        : keys(keys), seqNums(seqNums), valueSizes(valueSizes) {}