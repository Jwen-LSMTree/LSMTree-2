#ifndef OPTION_H
#define OPTION_H

#include <cstdint>

namespace Option
{
    extern const uint64_t SST_SPACE;
    extern const uint64_t BLOCK_SPACE;
    extern const uint64_t NON_ZERO_NUM;
    extern const uint64_t ZERO_SPACE;
    extern const uint64_t NON_ZERO_SPACES[];
    extern const uint64_t BLOOM_FILTER_SIZE;
    extern const char *const ZERO_NAME;
    extern const char *const NON_ZERO_NAMES[];

}

#endif
