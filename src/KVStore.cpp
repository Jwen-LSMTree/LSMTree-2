#include "../include/KVStore.h"
#include "../include/exception/NoEntryFound.h"

#include <string>

KVStore::KVStore(const string &dir) : KVStoreAPI(dir), disk(dir) {
    sequenceNumber = new SequenceNumber();
}

KVStore::~KVStore() {
    if (!mem.isEmpty()) {
        disk.flush(mem);
    }
}

void KVStore::put(uint64_t key, const string &value) {
    uint64_t seqNum = sequenceNumber->getUpdatedSeqNum();
    mem.put(key, value, seqNum);
    if (mem.space() >= Option::SST_SPACE) {
        disk.flush(mem);
        mem.clear();
    }
}

string KVStore::get(uint64_t key) {
    uint64_t seqNum = sequenceNumber->getSeqNum();
    try {
        return mem.get(key, seqNum);
    } catch (NoEntryFoundException &exception) {
        SearchResult result = disk.search(key, seqNum);
        if (result.success) {
            cout << "<disk search> key=" << key << ", value=" << result.value << endl;
            return result.value;
        }
        throw NoEntryFoundException("no entry found in DISK");
    }
}

string KVStore::getFromSnapshot(uint64_t key, uint64_t seqNum) {
    try {
        return mem.get(key, seqNum);
    } catch (NoEntryFoundException &exception) {
        SearchResult result = disk.search(key, seqNum);
        return result.value;
    }
}

void KVStore::reset() {
    mem.clear();
    disk.clear();
}

void KVStore::print() const {
    mem.print();
    disk.print();
}
