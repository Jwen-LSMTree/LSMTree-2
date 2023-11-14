#include "../../include/sequence_number_filter/SequenceNumberFilter.h"

SequenceNumberFilter::SequenceNumberFilter()
        : minSeqNum(std::numeric_limits<uint64_t>::max()) {
}

SequenceNumberFilter::~SequenceNumberFilter() = default;

bool SequenceNumberFilter::isVisible(uint64_t seqNum) const
{
    return (seqNum >= minSeqNum);
}