#include "KVStoreAPI.h"
#include "SequenceNumber.h"
#include "SkipList.h"
#include "DiskStorage.h"

#include <string>

class KVStore : public KVStoreAPI {
private:
    SequenceNumber *sequenceNumber;
    SkipList mem;
    DiskStorage disk;

public:
    explicit KVStore(const std::string &dir);

    ~KVStore();

    void put(uint64_t key, const std::string &value) override;

    string get(uint64_t key) override;

//	bool del(uint64_t key) override;

    void reset() override;
};
