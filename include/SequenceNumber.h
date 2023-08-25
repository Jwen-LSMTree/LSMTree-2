#ifndef SEQ_NUM_H
#define SEQ_NUM_H

#include <iostream>
#include <atomic>

using namespace std;

class SequenceNumber {
private:
    atomic<uint64_t> globalSeqNum = 0;

public:
    uint64_t getSeqNum();

    uint64_t getUpdatedSeqNum();
};

#endif
