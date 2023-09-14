#include "../../include/KVStore.h"

#include <filesystem>
#include <chrono>

using namespace std;

const int TEST_COUNT = 5;
const int ENTRY_COUNT = 10000000;

KVStore *store;
uint64_t keys[ENTRY_COUNT];
string values[ENTRY_COUNT];

void init();

int main() {
    auto totalTime = 0;
    for (int i = 0; i < TEST_COUNT; i++) {
        init();

        auto startTime = chrono::high_resolution_clock::now();
        for (int j = 0; j < ENTRY_COUNT; j++) {
            store->put(keys[j], values[j]);
        }
        auto endTime = chrono::high_resolution_clock::now();

        store->print();

        auto duration = chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count();
        cout << "Elapsed time: " << duration << " milliseconds" << "\n" << endl;
        totalTime += duration;
    }
    cout << "Average time: " << totalTime / TEST_COUNT << " milliseconds" << endl;
}

void init() {
    if (filesystem::exists(filesystem::path("./data"))) {
        filesystem::remove_all(filesystem::path("./data"));
    }
    store = new KVStore("./data");
    for (uint64_t i = 0; i < ENTRY_COUNT; ++i) {
        keys[i] = i;
        values[i] = "a";
    }
}
