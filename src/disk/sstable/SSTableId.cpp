#include "../../../include/disk/sstable/SSTableId.h"

#include <sstream>

SSTableId::SSTableId(const string &dir, uint64_t no)
        : dir(dir), no(no) {}

string SSTableId::name() const {
    ostringstream oss;
    oss << dir << "/" << no << ".sst";
    return oss.str();
}