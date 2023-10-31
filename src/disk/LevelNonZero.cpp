#include "../../include/disk/LevelNonZero.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <algorithm>

LevelNonZero::LevelNonZero(const string &dir) : dir(dir) {
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


SearchResult LevelNonZero::search(uint64_t key, uint64_t seqNum) const {
    for (const SSTable2 &sst: ssts) {
        SearchResult result = sst.search(key, seqNum);
        if (result.success) {
            return result;
        }
    }
    return false;
}

vector<Entry> LevelNonZero::flush() {
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

void LevelNonZero::merge(vector<Entry> &&lowerLevelEntries, uint64_t &id) {
    uint64_t minKey = lowerLevelEntries[0].key;
    uint64_t maxKey = lowerLevelEntries.back().key;

    // posit iterator on the first overlapped key
    auto itr = ssts.begin();
    while (itr != ssts.end() && itr->getMaxKey() < minKey) {
        itr++;
    }

    // collect every entry that is overlapped
    vector<Entry> curLevelEntries;
    while (itr != ssts.end() && itr->getMinKey() <= maxKey) {
        for (const Entry &entry: itr->load()) {
            curLevelEntries.emplace_back(entry);
        }
        byteCnt -= itr->space();
        itr->remove();
        itr = ssts.erase(itr);
        size--;
    }

    // compact
    vector<Entry> entries = Util::compact({curLevelEntries, lowerLevelEntries});

    // save
    size_t entryCnt = entries.size();
    size_t pos = 0;
    size_t i = 0;
    while (true) {
        vector<Entry> subEntries;
        while (subEntries.size() < Option::ENTRY_COUNT_PER_DATA_BLOCK && i < entryCnt) {
            subEntries.emplace_back(entries[i++]);
        }
        byteCnt += ssts.emplace(itr++, subEntries, pos, SSTableId(dir, id++))->space();
        size++;
        if (i == entryCnt) {
            break;
        }
    }
    save();
}

void LevelNonZero::clear() {
    while (!ssts.empty()) {
        ssts.pop_back();
    }
    size = 0;
    byteCnt = 0;
    save();
}

uint64_t LevelNonZero::space() const {
    return byteCnt;
}

void LevelNonZero::save() const {
    ofstream ofs(dir + "/index", ios::binary);
    ofs.write((char *) &size, sizeof(uint64_t));
    ofs.write((char *) &byteCnt, sizeof(uint64_t));
    for (const SSTable2 &sst: ssts) {
        uint64_t id = sst.getId();
        ofs.write((char *) &id, sizeof(uint64_t));
    }
    ofs.close();
}

void LevelNonZero::print(uint64_t i) const {
    cout << "=== LevelNonZero " << i + 1 << " === " << endl;
    uint64_t j = 0;
    for (const SSTable2 &sst: ssts) {
        sst.print(j++);
    }
}