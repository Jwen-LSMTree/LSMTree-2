#include "../../../include/disk/sstable/SSTableDataLocation.h"

SSTableDataLocation::SSTableDataLocation(vector<uint64_t> keys, vector<uint64_t> offsets, vector<uint64_t> seqNums,
                                         vector<uint64_t> oris)
        : keys(keys), offsets(offsets), seqNums(seqNums), dataBlockOffsets(oris){}
