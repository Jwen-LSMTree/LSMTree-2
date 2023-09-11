#include "../../include/disk/DiskStorage.h"

#include <filesystem>
#include <fstream>
#include <iostream>

DiskStorage::DiskStorage(const string &dir) : dir(dir), levelZero(dir + Option::LEVEL_ZERO_NAME) {
    if (filesystem::exists(filesystem::path(dir + "/meta"))) {
        ifstream ifs(dir + "/meta", ios::binary);
        ifs.read((char *) &id, sizeof(uint64_t));
        ifs.close();
    } else {
        id = 0;
        save();
    }
    for (uint64_t i = 0; i < Option::LEVEL_NON_ZERO_NUM; i++) {
        levelNonZeros.emplace_back(dir + Option::LEVEL_NON_ZERO_NAMES[i]);
    }
}

void DiskStorage::flush(const SkipList &mem) {
    levelZero.flushMemTable(mem, id);
    if (levelZero.space() > Option::LEVEL_ZERO_SPACE) {
        levelNonZeros[0].merge(levelZero.flush(), id);
    }
    for (uint64_t i = 0; i <= Option::LEVEL_NON_ZERO_NUM; i++) {
        if (levelNonZeros[i].space() > Option::LEVEL_NON_ZERO_SPACES[i]) {
            levelNonZeros[i + 1].merge(levelNonZeros[i].flush(), id);
        }
    }
    save();
}

SearchResult DiskStorage::search(uint64_t key, uint64_t seqNum) {
    SearchResult result = levelZero.search(key, seqNum);
    for (uint64_t i = 0; !result.success && i < Option::LEVEL_NON_ZERO_NUM; i++) {
        result = levelNonZeros[i].search(key, seqNum);
    }
    return result;
}

void DiskStorage::clear() {
    levelZero.clear();
    for (uint64_t i = 0; i < Option::LEVEL_NON_ZERO_NUM; i++) {
        levelNonZeros[i].clear();
    }
    id = 0;
    save();
}

void DiskStorage::save() const {
    ofstream ofs(dir + "/meta", ios::binary);
    ofs.write((char *) &id, sizeof(uint64_t));
    ofs.close();
}

LevelZero *DiskStorage::getLevelZero() {
    return &levelZero;
}

void DiskStorage::print() const {
    // LevelZero
    levelZero.print();

    // LevelNonZeros
    for (uint64_t i = 0; i < levelNonZeros.size(); i++) {
        levelNonZeros[i].print(i);
    }
}
