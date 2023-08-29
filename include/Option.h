#ifndef OPTION_H
#define OPTION_H

#include <cstdint>

namespace Option
{
    extern const bool COMPACTION;
    extern const uint64_t SST_SPACE;
    extern const uint64_t BLOCK_SPACE;
    extern const uint64_t NZ_NUM;
    extern const uint64_t Z_SPACE;
    extern const uint64_t NZ_SPACES[];
    extern const uint64_t BLOOM_FILTER_SIZE;
    extern const char *const Z_NAME;
    extern const char *const NZ_NAMES[];

}

#endif
