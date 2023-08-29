#include "../include/Option.h"

const bool Option::COMPACTION = true;

const uint64_t Option::SST_SPACE = (uint64_t) 2 * 1024 * 1024;

const uint64_t Option::BLOCK_SPACE = (uint64_t)4 * 1024;

const uint64_t Option::NZ_NUM = 6;

const uint64_t Option::Z_SPACE = (uint64_t)9 * 1024 * 1024;

const uint64_t Option::NZ_SPACES[] = {
    (uint64_t)64 * 1024 * 1024,
    (uint64_t)512 * 1024 * 1024,
    (uint64_t)4096 * 1024 * 1024,
    (uint64_t)32768 * 1024 * 1024,
    (uint64_t)262144 * 1024 * 1024,
    UINT64_MAX};

const uint64_t BLOOM_FILTER_SIZE = 1024;

const char *const Option::Z_NAME = "/L0";

const char *const Option::NZ_NAMES[] = {"/L1", "/L2", "/L3", "/L4", "/L5", "/L6"};
