#include "../../include/Option.h"

const uint64_t Option::SST_SPACE = (uint64_t) 1024;

const uint64_t Option::BLOCK_SPACE = (uint64_t) 2 * 1024;

const uint64_t Option::NON_ZERO_NUM = 6;

const uint64_t Option::ZERO_SPACE = (uint64_t) 2 * 1024;

const uint64_t Option::NON_ZERO_SPACES[] = {
        (uint64_t) 3 * 1024,
        (uint64_t) 4 * 1024,
        (uint64_t) 5 * 1024,
        (uint64_t) 6 * 1024,
        (uint64_t) 7 * 1024,
        UINT64_MAX};

const uint64_t Option::BLOOM_FILTER_SIZE = 1024;

const char *const Option::ZERO_NAME = "/L0";

const char *const Option::NON_ZERO_NAMES[] = {"/L1", "/L2", "/L3", "/L4", "/L5", "/L6"};
