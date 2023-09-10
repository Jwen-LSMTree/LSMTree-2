#include "../../include/disk/DiskStorage.h"

#include <filesystem>
#include <fstream>
#include <iostream>

DiskStorage::DiskStorage(const string &dir) : dir(dir), level0(dir + Option::LEVEL_ZERO_NAME) {
    if (filesystem::exists(filesystem::path(dir + "/meta"))) {
        ifstream ifs(dir + "/meta", ios::binary);
        ifs.read((char *) &no, sizeof(uint64_t));
        ifs.close();
    } else {
        no = 0;
        save();
    }
    for (uint64_t i = 0; i < Option::LEVEL_NON_ZERO_NUM; ++i)
        levels.emplace_back(dir + Option::LEVEL_NON_ZERO_NAMES[i]);
}

void DiskStorage::flush(const SkipList &mem) {
    level0.flush(mem, no);
    if (level0.space() > Option::LEVEL_ZERO_SPACE) {
        levels[0].merge(level0.extract(), no);
    }
    for (uint64_t i = 0; i + 1 < Option::LEVEL_NON_ZERO_NUM; ++i) {
        if (levels[i].space() > Option::LEVEL_NON_ZERO_SPACES[i]) {
            levels[i + 1].merge(levels[i].extract(), no);
        }
    }
    save();
}

SearchResult DiskStorage::search(uint64_t key, uint64_t seqNum) {
    SearchResult result = level0.search(key, seqNum);
    for (uint64_t i = 0; !result.success && i < Option::LEVEL_NON_ZERO_NUM; ++i)
        result = levels[i].search(key, seqNum);
    return result;
}

void DiskStorage::clear() {
    level0.clear();
    for (uint64_t i = 0; i < Option::LEVEL_NON_ZERO_NUM; ++i)
        levels[i].clear();
    no = 0;
    save();
}

void DiskStorage::save() const {
    ofstream ofs(dir + "/meta", ios::binary);
    ofs.write((char *) &no, sizeof(uint64_t));
    ofs.close();
}

LevelZero *DiskStorage::getLevelZero() {
    return &level0;
}

void DiskStorage::print() const {
    // LevelZero
    level0.print();

    // LevelNonZeros
    for (uint64_t i = 0; i < levels.size(); ++i) {
        levels[i].print(i);
    }
}
