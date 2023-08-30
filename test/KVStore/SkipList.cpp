#include "../../include/memory/SkipList.h"
#include "../../include/exception/NoEntryFound.h"

#include <gtest/gtest.h>

TEST(SkipList, get) {
    // given
    SkipList *mem = new SkipList();

    uint64_t key1 = 1;
    Entry *entry1_1 = new Entry(key1, "1", 1);
    Entry *entry1_2 = new Entry(key1, "2", 2);

    uint64_t key2 = 2;
    Entry *entry2_1 = new Entry(key2, "1", 3);
    Entry *entry2_2 = new Entry(key2, "2", 4);

    // when
    mem->put(entry1_1->key, entry1_1->value, entry1_1->seqNum);
    mem->put(entry1_2->key, entry1_2->value, entry1_2->seqNum);
    mem->put(entry2_1->key, entry2_1->value, entry2_1->seqNum);
    mem->put(entry2_2->key, entry2_2->value, entry2_2->seqNum);

    // then
    ASSERT_EQ(mem->get(key1, entry1_1->seqNum), entry1_1->value);
    ASSERT_EQ(mem->get(key1, entry1_2->seqNum), entry1_2->value);
    ASSERT_EQ(mem->get(key2, entry2_1->seqNum), entry2_1->value);
    ASSERT_EQ(mem->get(key2, entry2_2->seqNum), entry2_2->value);

    ASSERT_EQ(mem->get(key1, 5), entry1_2->value);
    ASSERT_EQ(mem->get(key2, 5), entry2_2->value);

    EXPECT_THROW(mem->get(0, 0), NoEntryFoundException);

    mem->print();
}
