#include "../../../include/disk/sstable/SSTableId.h"

#include <sstream>
#include <utility>

SSTableId::SSTableId(string dir, uint64_t id)
        : dir(std::move(dir)), id(id) {}

string SSTableId::name() const {
    ostringstream oss;
    oss << dir << "/" << id << ".sst";
    return oss.str();
}