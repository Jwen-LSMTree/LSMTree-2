#include "../include/KVStore.h"

#include <string>

KVStore::KVStore(const string &dir) : KVStoreAPI(dir), disk(dir) {
    sequenceNumber = new SequenceNumber();
}

KVStore::~KVStore() {
    if (!mem.empty()) {
        disk.add(mem);
    }
}

void KVStore::put(uint64_t key, const string &value) {
    uint64_t seqNum = sequenceNumber->getUpdatedSeqNum();
    mem.put(key, value, seqNum);
    if (mem.space() > Option::SST_SPACE) {
        disk.add(mem);
        mem.clear();
    }
}

string KVStore::get(uint64_t key) {
    uint64_t seqNum = sequenceNumber->getSeqNum();
    if (mem.contains(key)) {
        return mem.get(key);
    }
    SearchResult result = disk.search(key);
    return result.value;
}

//bool KVStore::del(uint64_t key) {
//    bool exist;
//    if (mem.contains(key))
//        exist = !mem.get(key).empty();
//    else {
//        SearchResult result = disk.search(key, false);
//        exist = result.success && result.location.len;
//    }
//    if (exist)
//        put(key, "");
//    return exist;
//}

void KVStore::reset() {
    mem.clear();
    disk.clear();
}
