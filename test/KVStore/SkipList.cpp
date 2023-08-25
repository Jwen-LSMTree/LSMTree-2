#include "../../include/memory/SkipList.h"

#include <gtest/gtest.h>

TEST(SkipList, put) {
    // given
    SkipList *mem = new SkipList();

    uint64_t key1 = 1;
    Entry *entry1_1 = new Entry(key1, "1", 1);
    Entry *entry1_2 = new Entry(key1, "2", 2);
    Entry *entry1_3 = new Entry(key1, "3", 3);
    Entry *entry1_4 = new Entry(key1, "4", 4);

    uint64_t key2 = 2;
    Entry *entry2_1 = new Entry(key2, "1", 5);

    // when
    cout << "--- put 1_1 ---" << endl;
    mem->put(entry1_1->key, entry1_1->value, entry1_1->seqNum);
    cout << "--- put 1_1 끝 ---\n" << endl;

    // 아래 주석을 해제하면 'key: 1, seqNum: 2'가 잘 저장되어 있는 것으로 출력됨
    // = 첫번째 put과 두번째 put에서의 key가 서로 다를 경우에는 정상 작동, 둘이 같은 key일 경우에는 첫번째로 저장했던 놈이 사라짐
//    cout << "--- put 2_1 ---" << endl;
//    mem->put(entry2_1->key, entry2_1->value, entry2_1->seqNum);
//    cout << "--- put 2_1 끝 ===\n" << endl;

    cout << "--- put 1_2 ---" << endl;
    mem->put(entry1_2->key, entry1_2->value, entry1_2->seqNum);
    cout << "--- put 1_2 끝 ===\n" << endl;

    cout << "--- put 1_3 ---" << endl;
    mem->put(entry1_3->key, entry1_3->value, entry1_3->seqNum);
    cout << "--- put 1_3 끝 ---\n" << endl;

    cout << "--- put 1_4 ---" << endl;
    mem->put(entry1_4->key, entry1_4->value, entry1_4->seqNum);
    cout << "--- put 1_4 끝 ---\n" << endl;
}
