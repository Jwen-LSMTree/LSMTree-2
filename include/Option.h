#ifndef OPTION_H
#define OPTION_H

#include <cstdint>

namespace Option {
    extern const uint64_t SST_SPACE;
    extern const uint64_t BLOCK_SPACE;
    extern const uint64_t ENTRY_COUNT_PER_DATA_BLOCK;
    extern const uint64_t LEVEL_NON_ZERO_NUM;
    extern const uint64_t LEVEL_ZERO_SPACE;
    extern const uint64_t LEVEL_NON_ZERO_SPACES[];
    extern const uint64_t BLOOM_FILTER_SIZE;
    extern const char *const LEVEL_ZERO_NAME;
    extern const char *const LEVEL_NON_ZERO_NAMES[];
}

#endif