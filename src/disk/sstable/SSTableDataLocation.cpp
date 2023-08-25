#include "../../../include/disk/sstable/SSTableDataLocation.h"

SSTableDataLocation::SSTableDataLocation(vector<uint64_t> keys, vector<uint64_t> offsets, vector<uint64_t> seqNums,
                                         vector<uint64_t> oris, vector<uint64_t> cmps)
        : keys(keys), offsets(offsets), seqNums(seqNums), oris(oris), cmps(cmps) {}
