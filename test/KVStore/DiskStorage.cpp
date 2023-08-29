#include "../../include/memory/SkipList.h"
#include "../../include/disk/DiskStorage.h"

#include <gtest/gtest.h>
#include <filesystem>

TEST(DiskStorage, search) {
    if (filesystem::exists(filesystem::path("./data"))) {
        filesystem::remove_all(filesystem::path("./data"));
    }

    // given
    SkipList *mem = new SkipList();

    uint64_t key1 = 1;
    Entry *entry1_1 = new Entry(key1, "1", 1);
    Entry *entry1_2 = new Entry(key1, "2", 2);

    uint64_t key2 = 2;
    Entry *entry2_1 = new Entry(key2, "1", 3);
    Entry *entry2_2 = new Entry(key2, "2", 4);

    mem->put(key1, entry1_1->value, entry1_1->seqNum);
    mem->put(key2, entry2_1->value, entry2_1->seqNum);
    mem->put(key1, entry1_2->value, entry1_2->seqNum);
    mem->put(key2, entry2_2->value, entry2_2->seqNum);

    // when
    DiskStorage *disk = new DiskStorage("./data");
    disk->add(*mem);
    LevelZero *level0 = disk->getLevelZero();

    // then
    ASSERT_EQ(level0->search(key1, entry1_1->seqNum).value, entry1_1->value);
    ASSERT_EQ(level0->search(key1, entry1_2->seqNum).value, entry1_2->value);
    ASSERT_EQ(level0->search(key2, entry2_1->seqNum).value, entry2_1->value);
    ASSERT_EQ(level0->search(key2, entry2_2->seqNum).value, entry2_2->value);

    ASSERT_EQ(level0->search(key1, 5).value, entry1_2->value);
    ASSERT_EQ(level0->search(key2, 5).value, entry2_2->value);

    ASSERT_EQ(level0->search(0, 0).success, false);
}
