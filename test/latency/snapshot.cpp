#include "../../include/KVStore.h"

#include <filesystem>
#include <chrono>

using namespace std;

const int ENTRY_COUNT = 10000;
const int DUPLICATE_COUNT = 3;
const string alphabets[5] = {"a", "b", "c", "d", "e"};

KVStore *store;
uint64_t keys[ENTRY_COUNT * DUPLICATE_COUNT];
string values[ENTRY_COUNT * DUPLICATE_COUNT];
uint64_t seqNums[ENTRY_COUNT * DUPLICATE_COUNT];

void init();

int main() {
    init();

    for (int i = 0; i < ENTRY_COUNT * DUPLICATE_COUNT; i++) {
        store->put(keys[i], values[i]);
    }
    store->print();

    auto startTime = chrono::high_resolution_clock::now();
    for (int i = 0; i < ENTRY_COUNT * DUPLICATE_COUNT; i++) {
        store->getFromSnapshot(keys[i], seqNums[i]);
    }
    auto endTime = chrono::high_resolution_clock::now();

    auto duration = chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count();
    cout << "Elapsed time: " << duration << " milliseconds" << "\n" << endl;
}

void init() {
    if (filesystem::exists(filesystem::path("./data"))) {
        filesystem::remove_all(filesystem::path("./data"));
    }
    store = new KVStore("./data");
    uint64_t seqNum = 0;
    for (uint64_t i = 0; i < DUPLICATE_COUNT; ++i) {
        for (uint64_t j = 0; j < ENTRY_COUNT; ++j) {
            keys[seqNum] = seqNum;
            values[seqNum] = alphabets[i];
            seqNums[seqNum] = ++seqNum;
        }
    }
}
