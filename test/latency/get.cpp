#include "../../include/KVStore.h"

#include <filesystem>
#include <chrono>

using namespace std;

int TEST_COUNT = 10;
int ENTRY_COUNT = 100000;

KVStore *store;
vector<uint64_t> keys;
vector<string> values;

void beforeEach();

void afterEach();

int main(int argc, char **argv) {
    if (argc >= 2) {
        ENTRY_COUNT = atoi(argv[1]);
    }
    if (argc >= 3) {
        TEST_COUNT = atoi(argv[2]);
    }

    auto totalTime = 0;
    for (int i = 0; i < TEST_COUNT; i++) {
        beforeEach();

        for (int j = 0; j < ENTRY_COUNT; j++) {
            store->put(keys[j], values[j]);
        }
//        store->print();

        auto startTime = chrono::high_resolution_clock::now();
        for (unsigned long long key: keys) {
            store->get(key);
        }
        auto endTime = chrono::high_resolution_clock::now();

        auto duration = chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count();
        cout << "Elapsed time: " << duration << " milliseconds" << "\n" << endl;
        totalTime += duration;

        afterEach();
    }
    cout << "Average time: " << totalTime / TEST_COUNT << " milliseconds" << endl;
}

void beforeEach() {
    if (filesystem::exists(filesystem::path("./data"))) {
        filesystem::remove_all(filesystem::path("./data"));
    }
    store = new KVStore("./data");

    keys.clear();
    values.clear();
    keys.resize(ENTRY_COUNT);
    values.resize(ENTRY_COUNT);

    for (uint64_t i = 0; i < ENTRY_COUNT; ++i) {
        keys[i] = i;
        values[i] = "a";
    }
}

void afterEach() {
    store->reset();
    delete store;
    store = nullptr;
}
