#include "../../include/KVStore.h"

#include <gtest/gtest.h>
#include <filesystem>
#include <chrono>

const uint64_t ENTRY_COUNT = 10000;
uint64_t keys[ENTRY_COUNT];
string values[ENTRY_COUNT];

void generateKVs();

void generateKVs() {
    for (uint64_t i = 0; i < ENTRY_COUNT; ++i) {
        keys[i] = i;
        values[i] = "a";
    }
}

TEST(KVStore, put) {
    if (filesystem::exists(filesystem::path("./data"))) {
        filesystem::remove_all(filesystem::path("./data"));
    }

    // given
    auto store = new KVStore("./data");
    generateKVs();

    // when
    auto startTime = chrono::high_resolution_clock::now();
    for (int i = 0; i < ENTRY_COUNT; ++i) {
        store->put(keys[i], values[i]);
    }
    auto endTime = chrono::high_resolution_clock::now();

    // then
    store->print();
    auto duration = chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count();
    cout << "Elapsed time: " << duration << " milliseconds" << "\n" << endl;
}

TEST(KVStore, get) {
    if (filesystem::exists(filesystem::path("./data"))) {
        filesystem::remove_all(filesystem::path("./data"));
    }

    // given
    auto store = new KVStore("./data");
    generateKVs();

    // when
    for (int i = 0; i < ENTRY_COUNT; ++i) {
        store->put(keys[i], values[i]);
    }

    // then
    for (int i = 0; i < ENTRY_COUNT; ++i) {
        ASSERT_EQ(store->get(keys[i]), values[i]);
    }
}

TEST(KVStore, compaction) {
    if (filesystem::exists(filesystem::path("./data"))) {
        filesystem::remove_all(filesystem::path("./data"));
    }

    // given
    auto store = new KVStore("./data");
    generateKVs();

    for (int i = 1; i < 39; ++i) {
        store->put(keys[i], values[i]);
    }
    store->put(keys[39], values[39]);
    cout << "\n1" << endl;
    store->print();

    // Disk Level NonZero에 2개의 SSTable
    for (int i = 40; i < 77; ++i) {
        store->put(keys[i], values[i]);
    }
    cout << "\n2" << endl;
    store->print();

    // Disk Level NonZero에 3개의 SSTable이 들어가려다가 못들어가므로 compaction이 일어남
    for (int i = 19; i < 57; ++i) {
        store->put(keys[i], values[i]);
    }
    cout << "\n3" << endl;
    store->print();
}
