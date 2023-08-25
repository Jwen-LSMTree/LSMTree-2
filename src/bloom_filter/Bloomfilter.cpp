#include "../../include/bloom_filter/BloomFilter.h"

// 생성자, BLOOM_FILTER_SIZE만큼의 비트배열 생성 후 0으로 초기화
BloomFilter::BloomFilter() {
    byteArray = new bool[BLOOM_FILTER_SIZE];
    byteArray = (bool *) memset(byteArray, 0, BLOOM_FILTER_SIZE);
}

// 생성자, BLOOM_FILTER_SIZE만큼의 비트배열 생성 후 인자로 받은 bits로 초기화
BloomFilter::BloomFilter(bool *bits) {
    byteArray = new bool[BLOOM_FILTER_SIZE];
    memcpy(byteArray, bits, BLOOM_FILTER_SIZE);
}

BloomFilter::~BloomFilter() = default;

// key값이 들어오면 hashValues(4개) 계산 후 비트배열에 표시
void BloomFilter::insert(uint64_t k) {
    uint32_t *hashValues = new uint32_t[4];
    hashValues = (uint32_t *) memset(hashValues, 0, 4 * sizeof(uint32_t));
    MurmurHash3_x64_128(&k, sizeof(k), 1, hashValues);
    for (int i = 0; i < 4; ++i) {
        int index = *(hashValues + i) % BLOOM_FILTER_SIZE;
        byteArray[index] = 1;
    }
}

// 하나라도 false면 없는 키로 판단
bool BloomFilter::hasKey(uint64_t k) const {
    uint32_t *hashValues = new uint32_t[4];
    hashValues = (uint32_t *) memset(hashValues, 0, 4 * sizeof(uint32_t));
    MurmurHash3_x64_128(&k, sizeof(k), 1, hashValues);
    return byteArray[*hashValues % BLOOM_FILTER_SIZE] & byteArray[*(hashValues + 1) % BLOOM_FILTER_SIZE]
           & byteArray[*(hashValues + 2) % BLOOM_FILTER_SIZE] & byteArray[*(hashValues + 3) % BLOOM_FILTER_SIZE];
}
