#include "../include/Entry.h"

#include <utility>

Entry::Entry(uint64_t key, string value)
        : key(key), value(std::move(value)) {}

Entry::Entry(uint64_t key, string value, uint64_t seqNum)
        : key(key), value(std::move(value)), seqNum(seqNum) {}
