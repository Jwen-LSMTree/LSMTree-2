#include "SequenceNumber.h"

#include <gtest/gtest.h>
#include <pthread.h>
#include <vector>

void *read(void *threadId);

void *write(void *threadId);

TEST(SequenceNumber, seqNoTest) {
    // given
    auto *s = new SequenceNumber();

    // when
    uint64_t seqNo = s->getSeqNum();
    uint64_t updatedSeqNo = s->getUpdatedSeqNum();

    // then
    ASSERT_EQ(seqNo, 0);
    ASSERT_EQ(updatedSeqNo, 1);
}

auto *s = new SequenceNumber();
int THREAD_COUNT = 32;
int OPERATION_COUNT = 50;
int DIVIDER = 4;
int WRITE_COUNT = (THREAD_COUNT / DIVIDER) * (DIVIDER - 1);

TEST(SequenceNumber, multi_threaded_seqNoTest) {
    // given
    pthread_t threads[THREAD_COUNT];
    uint64_t initSeqNo = s->getSeqNum();

    // when
    for (int i = 0; i < THREAD_COUNT; i++) {
        if (i % DIVIDER == 0) {
            pthread_create(&threads[i], nullptr, &read, (void *) (long) i);
            continue;
        }
        pthread_create(&threads[i], nullptr, &write, (void *) (long) i);
    }
    for (auto &thread: threads) {
        pthread_join(thread, nullptr);
    }

    // then
    uint64_t seqNo = s->getSeqNum();
    ASSERT_EQ(seqNo, initSeqNo + WRITE_COUNT * OPERATION_COUNT);
}

void *read(void *threadId) {
    for (int i = 0; i < OPERATION_COUNT; ++i) {
        uint64_t seqNo = s->getSeqNum();
        cout << "[read  | seqNo: " << seqNo << "]\n";
        usleep(1000);
    }
    return nullptr;
}

void *write(void *threadId) {
    for (int i = 0; i < OPERATION_COUNT; ++i) {
        uint64_t seqNo = s->getUpdatedSeqNum();
        cout << "[write | seqNo: " << seqNo << "]\n";
        usleep(1000);
    }
    return nullptr;
}
