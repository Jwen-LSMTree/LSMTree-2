//
// Created by 김정우 on 2023/10/22.
//
#include "../../include/KVStore.h"
#include "../../include/memory/SkipList.h"
#include "../../include/exception/NoEntryFound.h"

#include <gtest/gtest.h>
#include <filesystem>

const uint64_t ENTRY_COUNT = 10000;
uint64_t keys[ENTRY_COUNT];
string values[ENTRY_COUNT];

void generateKVs() {
    for (uint64_t i = 0; i < ENTRY_COUNT; ++i) {
        keys[i] = i;
        values[i] = "a";
    }
}

TEST(KVStore, sequence) {
    if (filesystem::exists(filesystem::path("./data"))) {
        filesystem::remove_all(filesystem::path("./data"));
    }

    // given
    auto store = new KVStore("./data");
    generateKVs();
    ASSERT_EQ(store->sequenceNumber->getSeqNum(), 0);

    store->put(1, "a");
    ASSERT_EQ(store->sequenceNumber->getSeqNum(), 1);

    store->get(1);
    ASSERT_EQ(store->sequenceNumber->getSeqNum(), 1);

}


TEST(KVStore, put2) {
    if (filesystem::exists(filesystem::path("./data"))) {
        filesystem::remove_all(filesystem::path("./data"));
    }

    // given
    auto store = new KVStore("./data");
    generateKVs();

    for (int i = 1; i < 38; ++i) {
        store->put(keys[i], values[i]);
    }
    ASSERT_EQ(store->mem.size(), 37);

    store->put(keys[38], "a");
    ASSERT_EQ(store->mem.size(), 0);
    store->print();
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

    store->print();

    // Disk Level NonZero에 2개의 SSTable
    for (int i = 40; i < 77; ++i) {
        store->put(keys[i], values[i]);
    }

    store->print();


    // Disk Level NonZero에 3개의 SSTable이 들어가려다가 못들어가므로 compaction이 일어남
    for (int i = 19; i < 57; ++i) {
        store->put(keys[i], values[i]);
    }

    store->print();
}

TEST(KVStore, get) {
    if (filesystem::exists(filesystem::path("./data"))) {
        filesystem::remove_all(filesystem::path("./data"));
    }

    // given
    auto store = new KVStore("./data");
    generateKVs();

    for (int i = 1; i < 77; ++i) {
        store->put(keys[i], values[i]);
    }


    // Disk Level NonZero에 2개의 SSTable
    for (int i = 1; i < 77; ++i) {
        store->put(keys[i], "b");
    }

    for (int i = 1; i < 77; ++i) {
        ASSERT_EQ(store->get(keys[i]), "b");
    }

    store->put(1, "changed");
    ASSERT_EQ(store->get(1), "changed");

}

TEST(BloomFilter, hasKey) {
    if (filesystem::exists(filesystem::path("./data"))) {
        filesystem::remove_all(filesystem::path("./data"));
    }

    // given
    SkipList skipList = SkipList();
    skipList.put(1, "a", 1);
    skipList.put(2, "b", 2);
    skipList.put(3, "c", 3);

    auto s1 = new SSTable(skipList, SSTableId("./data/1.sst",1));
    ASSERT_TRUE(s1->bloomfilter.hasKey(1));
    ASSERT_TRUE(s1->bloomfilter.hasKey(2));
    ASSERT_TRUE(s1->bloomfilter.hasKey(3));
}

TEST(KVStore,notFound) {
    if (filesystem::exists(filesystem::path("./data"))) {
        filesystem::remove_all(filesystem::path("./data"));
    }

    auto store = new KVStore("./data");
    for (int i = 1; i < 4; ++i) {
        store->put(i, "a");
    }

    ASSERT_EQ(store->get(1), "a");
    ASSERT_EQ(store->get(2), "a");
    ASSERT_EQ(store->get(3), "a");

    ASSERT_THROW(store->get(4), NoEntryFoundException);
    ASSERT_THROW(store->get(5), NoEntryFoundException);
    ASSERT_THROW(store->get(6), NoEntryFoundException);
}

TEST(KVStore,snapshot) {
    if (filesystem::exists(filesystem::path("./data"))) {
        filesystem::remove_all(filesystem::path("./data"));
    }

    auto store = new KVStore("./data");
    store->put(1, "a");
    store->put(2, "a");
    store->put(3, "a");
    store->put(1, "b");
    store->put(2, "b");
    store->put(3, "b");

    ASSERT_EQ(store->getFromSnapshot(1, 1), "a");
    ASSERT_EQ(store->getFromSnapshot(2, 2), "a");
    ASSERT_EQ(store->getFromSnapshot(3, 3), "a");
    ASSERT_EQ(store->getFromSnapshot(1, 4), "b");
    ASSERT_EQ(store->getFromSnapshot(2, 5), "b");
    ASSERT_EQ(store->getFromSnapshot(3, 6), "b");
}