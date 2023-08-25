#include "../include/SequenceNumber.h"

uint64_t SequenceNumber::getSeqNum() {
    return globalSeqNum.load(memory_order_acquire);
}

uint64_t SequenceNumber::getUpdatedSeqNum() {
    while (true) {
        uint64_t globalSeqNum_expected = getSeqNum();
        if (globalSeqNum.compare_exchange_strong(globalSeqNum_expected,
                                                 globalSeqNum_expected + 1,
                                                 memory_order_acquire)) {
            return globalSeqNum_expected + 1;
        }
    }
}
