#include "SSTableDataLocation.h"

SSTableDataLocation::SSTableDataLocation(std::vector<uint64_t> keys, std::vector<uint64_t> offsets, std::vector<uint64_t> oris ,std::vector<uint64_t> cmps)
:keys(keys), offsets(offsets), oris(oris), cmps(cmps) {}
