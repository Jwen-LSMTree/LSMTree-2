#ifndef KV_STORE_API_H
#define KV_STORE_API_H

#include "KVStoreAPI.h"
#include "SequenceNumber.h"
#include "memory/SkipList.h"
#include "disk/DiskStorage.h"
#include "disk/SearchResult.h"
#include "Option.h"

#include <string>

using namespace std;

class KVStore : public KVStoreAPI {
private:
    SequenceNumber *sequenceNumber;
    SkipList mem;
    DiskStorage disk;

public:
    explicit KVStore(const string &dir);

    ~KVStore();

    void put(uint64_t key, const string &value) override;

    string get(uint64_t key) override;

    string getFromSnapshot(uint64_t key, uint64_t seqNum);

//	bool del(uint64_t key) override;

    void reset() override;
};

#endif
