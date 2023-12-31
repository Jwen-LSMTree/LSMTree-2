#include "../../include/KVStore.h"

#include <filesystem>
#include <chrono>

using namespace std;

int TEST_COUNT = 2;
int ENTRY_COUNT = 1000000;
int DUPLICATE_COUNT = 3;

const string alphabets[7] = {"a", "b", "c", "d", "e", "f", "g"};

KVStore *store;
vector<uint64_t> keys;
vector<string> values;
vector<uint64_t> seqNums;

void beforeEach();

void afterEach();

int main(int argc, char **argv) {
    if (argc >= 2) {
        ENTRY_COUNT = atoi(argv[1]);
    }
    if (argc >= 3) {
        DUPLICATE_COUNT = atoi(argv[2]);
        if (DUPLICATE_COUNT > 7) {
            DUPLICATE_COUNT = 7;
        }
    }
    if (argc >= 4) {
        TEST_COUNT = atoi(argv[3]);
    }

    uint64_t totalTime = 0;
    uint64_t totalLatency = 0;
    for (int i = 0; i < TEST_COUNT; i++) {
        beforeEach();

        for (int j = 0; j < ENTRY_COUNT * DUPLICATE_COUNT; j++) {
            store->put(keys[j], values[j]);
        }
//        store->print();

        auto startTime = chrono::high_resolution_clock::now();
        for (int j = 0; j < ENTRY_COUNT * DUPLICATE_COUNT; j++) {
            store->getFromSnapshot(keys[j], seqNums[j]);
        }
        auto endTime = chrono::high_resolution_clock::now();

        auto duration = chrono::duration_cast<chrono::nanoseconds>(endTime - startTime).count();
        cout << "Elapsed time: " << duration << " nanoseconds" << endl;
        totalTime += duration;

        auto latency = chrono::duration_cast<chrono::nanoseconds>(endTime - startTime).count() /
                       (ENTRY_COUNT * DUPLICATE_COUNT);
        cout << "Latency: " << latency << " nanoseconds" << "\n" << endl;
        totalLatency += latency;

        afterEach();
    }
    cout << "Average time: " << totalTime / TEST_COUNT << " nanoseconds" << endl;
    cout << "Average latency: " << totalLatency / TEST_COUNT << " nanoseconds" << endl;
}

void beforeEach() {
    if (filesystem::exists(filesystem::path("./data"))) {
        filesystem::remove_all(filesystem::path("./data"));
    }
    store = new KVStore("./data");

    keys.clear();
    values.clear();
    seqNums.clear();
    keys.resize(ENTRY_COUNT * DUPLICATE_COUNT);
    values.resize(ENTRY_COUNT * DUPLICATE_COUNT);
    seqNums.resize(ENTRY_COUNT * DUPLICATE_COUNT);

    uint64_t seqNum = 0;
    for (uint64_t i = 0; i < DUPLICATE_COUNT; ++i) {
        for (uint64_t j = 0; j < ENTRY_COUNT; ++j) {
            keys[seqNum] = seqNum;
            values[seqNum] = alphabets[i];
            seqNums[seqNum] = seqNum + 1;
            seqNum++;
        }
    }
}

void afterEach() {
    store->reset();
    delete store;
    store = nullptr;
}
