#include "../../include/disk/LevelNonZero.h"

#include <filesystem>
#include <fstream>
#include <iostream>

LevelNonZero::LevelNonZero(const string &dir) : dir(dir) {
    if (!filesystem::exists(filesystem::path(dir))) {
        filesystem::create_directories(filesystem::path(dir));
        size = 0;
        byteCnt = 0;
        lastKey = 0;
        save();
    } else {
        ifstream ifs(dir + "/index", ios::binary);
        ifs.read((char *) &size, sizeof(uint64_t));
        ifs.read((char *) &byteCnt, sizeof(uint64_t));
        ifs.read((char *) &lastKey, sizeof(uint64_t));
        for (uint64_t i = 0; i < size; ++i) {
            uint64_t no;
            ifs.read((char *) &no, sizeof(uint64_t));
            ssts.emplace_back(SSTableId(dir, no));
        }
        ifs.close();
    }
}

SearchResult LevelNonZero::search(uint64_t key, uint64_t seqNum) const {
    for (const SSTable &sst: ssts) {
        SearchResult res = sst.search(key, seqNum);
        if (res.success)
            return res;
    }
    return false;
}

vector<Entry> LevelNonZero::extract() {
    auto itr = ssts.begin();
    while (itr != ssts.end() && itr->upper() <= lastKey)
        ++itr;
    if (itr == ssts.end())
        itr = ssts.begin();
    byteCnt -= itr->space();
    lastKey = itr->upper();
    vector<Entry> ret = itr->load();
    itr->remove();
    ssts.erase(itr);
    --size;
    save();
    return ret;
}

void LevelNonZero::merge(vector<Entry> &&entries1, uint64_t &no) {
    uint64_t lo = entries1[0].key;
    uint64_t hi = entries1.back().key;

    vector<Entry> entries0;
    auto itr = ssts.begin();
    while (itr != ssts.end() && itr->upper() < lo) ++itr;

    while (itr != ssts.end() && itr->lower() <= hi) {
        for (const Entry &entry: itr->load()) {
            entries0.emplace_back(entry);
        }
        byteCnt -= itr->space();
        itr->remove();
        itr = ssts.erase(itr);
        --size;
    }
    vector<Entry> entries = Util::compact({entries0, entries1});
    size_t n = entries.size();
    size_t pos = 0;
    while (pos < n) {
        byteCnt += ssts.emplace(itr, entries, pos, SSTableId(dir, no++))->space();
        ++size;
    }
    save();
}

void LevelNonZero::clear() {
    while (!ssts.empty()) {
        ssts.back().remove();
        ssts.pop_back();
    }
    size = 0;
    byteCnt = 0;
    lastKey = 0;
    save();
}

uint64_t LevelNonZero::space() const {
    return byteCnt;
}

void LevelNonZero::save() const {
    ofstream ofs(dir + "/index", ios::binary);
    ofs.write((char *) &size, sizeof(uint64_t));
    ofs.write((char *) &byteCnt, sizeof(uint64_t));
    ofs.write((char *) &lastKey, sizeof(uint64_t));
    for (const SSTable &sst: ssts) {
        uint64_t no = sst.number();
        ofs.write((char *) &no, sizeof(uint64_t));
    }
    ofs.close();
}

void LevelNonZero::print(uint64_t i) const {
    cout << "=== LevelNonZero " << i << " === " << endl;
    uint64_t j = 0;
    for (const SSTable &sst: ssts) {
        sst.print(j++);
    }
}
