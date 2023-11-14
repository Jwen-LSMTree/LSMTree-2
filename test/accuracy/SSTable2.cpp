//
// Created by 김 정우 on 10/28/23.
//
#include "../../include/memory/SkipList.h"
#include "../../include/disk/DiskStorage.h"
#include "../../include/disk/sstable/SSTable2.h"

#include <gtest/gtest.h>
#include <filesystem>

TEST(SSTable2, find) {
    if (filesystem::exists(filesystem::path("./data"))) {
        filesystem::remove_all(filesystem::path("./data"));
    }

    // given
    SkipList *mem = new SkipList();

    uint64_t key1 = 1;
    Entry *entry1_1 = new Entry(key1, "a", 1);

    mem->put(key1, entry1_1->value, entry1_1->seqNum);

    // when
    SSTable2 *sstable = new SSTable2(*mem, SSTableId("./data/0.sst",0));

    // then
    ASSERT_EQ(sstable->search(key1, entry1_1->seqNum).value, entry1_1->value);
}