//
// Created by 김 정우 on 10/28/23.
//

#include "../../../include/disk/sstable/DataBlockLocation.h"

DataBlockLocation::DataBlockLocation(vector<uint64_t> minKeys, vector<uint64_t> dataBlockOffsets)
        : minKeys(minKeys), dataBlockOffsets(dataBlockOffsets) {}