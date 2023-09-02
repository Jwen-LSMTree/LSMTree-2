#include "../../include/sequence_number_filter/Sequence_number_filter.h"

SequenceNumberFilter::SequenceNumberFilter()
        : minSeqNum(std::numeric_limits<uint64_t>::max()), maxSeqNum(std::numeric_limits<uint64_t>::min()) {
}

SequenceNumberFilter::~SequenceNumberFilter() = default;

bool SequenceNumberFilter::isVisible(uint64_t seqNum) const
{
    return (seqNum >= minSeqNum) && (seqNum <= maxSeqNum);
}