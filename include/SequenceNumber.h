#ifndef LSMTREE_SEQ_NO_H
#define LSMTREE_SEQ_NO_H

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
