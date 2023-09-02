#ifndef LSMTREE_SEQUENCE_NUMBER_FILTER_H
#define LSMTREE_SEQUENCE_NUMBER_FILTER_H

#include "../SequenceNumber.h"
#include <limits>

class SequenceNumberFilter{
public:
    SequenceNumberFilter();

    ~SequenceNumberFilter();

    uint64_t minSeqNum;

    uint64_t maxSeqNum;

    bool isVisible(uint64_t seqNum) const;
};

#endif //LSMTREE_SEQUENCE_NUMBER_FILTER_H
