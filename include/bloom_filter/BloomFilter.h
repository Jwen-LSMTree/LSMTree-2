#ifndef LSM_TREE_BLOOMFILTER_H
#define LSM_TREE_BLOOMFILTER_H

#include "Murmurhash3.h"
#include "../Option.h"
#include <cstring>

class BloomFilter
{
public:
    bool *byteArray;

    BloomFilter();

    explicit BloomFilter(bool *byteArray);

    ~BloomFilter();

    bool hasKey(uint64_t key) const;

    void insert(uint64_t key);
};

#endif // LSM_TREE_BLOOMFILTER_H