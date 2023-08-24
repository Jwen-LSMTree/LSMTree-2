#ifndef SSTABLE_DATA_LOCATION_H
#define SSTABLE_DATA_LOCATION_H

#include <vector>

class SSTableDataLocation
{
public:
    std::vector<uint64_t> keys;
    std::vector<uint64_t> offsets;
    std::vector<uint64_t> oris;
    std::vector<uint64_t> cmps;
    SSTableDataLocation(std::vector<uint64_t> keys, std::vector<uint64_t> offsets, std::vector<uint64_t> oris ,std::vector<uint64_t> cmps);
    SSTableDataLocation() = default;
private:
    /* data */

};

#endif