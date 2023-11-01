#include "../../include/disk/LevelZero.h"

#include <filesystem>
#include <fstream>
#include <iostream>

LevelZero::LevelZero(const string &dir) : dir(dir) {
    if (!filesystem::exists(filesystem::path(dir))) {
        filesystem::create_directories(filesystem::path(dir));
        size = 0;
        byteCnt = 0;
        save();
        return;
    }
    ifstream ifs(dir + "/index", ios::binary);
    ifs.read((char *) &size, sizeof(uint64_t));
    ifs.read((char *) &byteCnt, sizeof(uint64_t));
    for (uint64_t i = 0; i < size; i++) {
        uint64_t id;
        ifs.read((char *) &id, sizeof(uint64_t));
        ssts.emplace_back(SSTableId(dir, id));
    }
    ifs.close();
}

SearchResult LevelZero::search(uint64_t key, uint64_t seqNum) const {
    for (uint64_t i = 1; i <= size; i++) {
        SearchResult result = ssts[size - i].search(key, seqNum);
        if (result.success) {
            return result;
        }
    }
    return false;
}

void LevelZero::flushMemTable(const SkipList &mem, uint64_t &id) {
    ssts.emplace_back(mem, SSTableId(dir, id++));
    size++;
    byteCnt += mem.space();
    save();
}

vector<Entry> LevelZero::flush() {
    uint64_t minSeqNum = numeric_limits<uint64_t>::max();
    uint64_t sstIdx = -1;

    int i = 0;
    for (const SSTable2 &sst: ssts) {
        if (sst.getMinSeqNum() < minSeqNum) {
            minSeqNum = sst.getMinSeqNum();
            sstIdx = i;
        }
        i++;
    }

    auto itr = ssts.begin();
    advance(itr, sstIdx);

    vector<Entry> entries = itr->load();
    byteCnt -= itr->space();
    itr->remove();
    ssts.erase(itr);
    size--;
    save();

    return entries;
}

void LevelZero::clear() {
    while (!ssts.empty()) {
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
    for (const SSTable2 &sst: ssts) {
        uint64_t id = sst.getId();
        ofs.write((char *) &id, sizeof(uint64_t));
    }
    ofs.close();
}

void LevelZero::print() const {
    cout << "=== LevelZero === " << endl;
    for (uint64_t i = 0; i < ssts.size(); i++) {
        ssts[i].print(i);
    }
}
