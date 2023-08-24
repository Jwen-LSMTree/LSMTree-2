#include "LevelZero.h"
#include "SSTableId.h"
#include "Util.h"

#include <filesystem>
#include <fstream>
#include <iostream>

LevelZero::LevelZero(const string &dir, TableCache *tableCache) : dir(dir), tableCache(tableCache) {
    if (!filesystem::exists(filesystem::path(dir))) {
        filesystem::create_directories(filesystem::path(dir));
        size = 0;
        byteCnt = 0;
        save();
    } else {
        ifstream ifs(dir + "/index", ios::binary);
        ifs.read((char *) &size, sizeof(uint64_t));
        ifs.read((char *) &byteCnt, sizeof(uint64_t));
        for (uint64_t i = 0; i < size; ++i) {
            uint64_t no;
            ifs.read((char *) &no, sizeof(uint64_t));
            ssts.emplace_back(SSTableId(dir, no), tableCache);
        }
        ifs.close();
    }
}

SearchResult LevelZero::search(uint64_t key) const {
    for (uint64_t i = 1; i <= size; ++i) {
        SearchResult res = ssts[size - i].search(key);
        if (res.success)
            return res;
    }
    return false;
}

void LevelZero::add(const SkipList &mem, uint64_t &no) {
    ssts.emplace_back(mem, SSTableId(dir, no++), tableCache);
    ++size;
    byteCnt += mem.space();
    save();
}

vector<Entry> LevelZero::extract() {
    vector<vector<Entry>> inputs;
    for (const SSTable &sst: ssts) {
        inputs.emplace_back(sst.load());
        sst.remove();
    }
    size = 0;
    byteCnt = 0;
    ssts.clear();
    save();
    return Util::compact(inputs);
}

void LevelZero::clear() {
    while (!ssts.empty()) {
        ssts.back().remove();
        ssts.pop_back();
    }
    size = 0;
    byteCnt = 0;
    save();
}

uint64_t LevelZero::space() const {
    return byteCnt;
}

void LevelZero::save() const {
    ofstream ofs(dir + "/index", ios::binary);
    ofs.write((char *) &size, sizeof(uint64_t));
    ofs.write((char *) &byteCnt, sizeof(uint64_t));
    for (const SSTable &sst: ssts) {
        uint64_t no = sst.number();
        ofs.write((char *) &no, sizeof(uint64_t));
    }
    ofs.close();
}