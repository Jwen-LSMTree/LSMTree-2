#include "../../include/bloom_filter/BloomFilter.h"

BloomFilter::BloomFilter()
{
    byteArray = new bool[Option::BLOOM_FILTER_SIZE];
    byteArray = (bool *)memset(byteArray, 0, Option::BLOOM_FILTER_SIZE);
}

BloomFilter::BloomFilter(bool *bits)
{
    byteArray = new bool[Option::BLOOM_FILTER_SIZE];
    memcpy(byteArray, bits, Option::BLOOM_FILTER_SIZE);
}

BloomFilter::~BloomFilter() = default;

void BloomFilter::insert(uint64_t k)
{
    uint32_t *hashValues = new uint32_t[4];
    hashValues = (uint32_t *)memset(hashValues, 0, 4 * sizeof(uint32_t));
    MurmurHash3_x64_128(&k, sizeof(k), 1, hashValues);
    for (int i = 0; i < 4; ++i)
    {
        int index = *(hashValues + i) % Option::BLOOM_FILTER_SIZE;
        byteArray[index] = 1;
    }
}

bool BloomFilter::hasKey(uint64_t k) const
{
    uint32_t *hashValues = new uint32_t[4];
    hashValues = (uint32_t *)memset(hashValues, 0, 4 * sizeof(uint32_t));
    MurmurHash3_x64_128(&k, sizeof(k), 1, hashValues);
    return byteArray[*hashValues % Option::BLOOM_FILTER_SIZE] & byteArray[*(hashValues + 1) % Option::BLOOM_FILTER_SIZE] & byteArray[*(hashValues + 2) % Option::BLOOM_FILTER_SIZE] & byteArray[*(hashValues + 3) % Option::BLOOM_FILTER_SIZE];
}
