#include "SSTable.h"
#include "Util.h"
#include "Option.h"
// #include "snappy.h"

#include <fstream>
#include <filesystem>

SSTable::SSTable(const SSTableId &id, TableCache *tableCache)
        : id(id), tableCache(tableCache) {
    ifstream ifs(id.name(), ios::binary);

    // EntryIndex
    ifs.read((char *) &entryCnt, sizeof(uint64_t)); // n
    for (uint64_t i = 0; i <= entryCnt; ++i) {
        uint64_t key, offset, seqNum;
        ifs.read((char *) &key, sizeof(uint64_t));
        ifs.read((char *) &offset, sizeof(uint64_t));
        ifs.read((char *) &seqNum, sizeof(uint64_t));
        keys.push_back(key);
        offsets.push_back(offset);
        seqNums.push_back(seqNum);
    }

    // BlockIndex
    ifs.read((char *) &blockCnt, sizeof(uint64_t)); // m
    for (uint64_t i = 0; i <= blockCnt; ++i) {
        uint64_t ori, cmp;
        ifs.read((char *) &ori, sizeof(uint64_t));
        ifs.read((char *) &cmp, sizeof(uint64_t));
        oris.push_back(ori);
        cmps.push_back(cmp);
    }
    ifs.close();
}

SSTable::SSTable(const SkipList &mem, const SSTableId &id, TableCache *tableCache)
        : id(id), tableCache(tableCache) {
    entryCnt = mem.size();
    blockCnt = 0;
    uint64_t offset = 0;
    uint64_t ori = 0;
    uint64_t cmp = 0;
    string block;
    block.reserve(Option::BLOCK_SPACE);
    string blockSeg;
    blockSeg.reserve(Option::SST_SPACE);
    uint64_t entryInBlockCnt = 0;

    SkipList::Iterator itr = mem.iterator();
    while (itr.hasNext()) {
        Entry entry = itr.next();
        keys.push_back(entry.key);
        offsets.push_back(offset);
        seqNums.push_back(entry.seqNum);

        offset += entry.value.size();
        block += entry.value;
        ++entryInBlockCnt;

        if (block.size() >= Option::BLOCK_SPACE) {
            string compressed;
            compressed = block;
            blockSeg += compressed;
            oris.push_back(ori);
            cmps.push_back(cmp);
            ori += block.size();
            cmp += compressed.size();
            block.clear();
            entryInBlockCnt = 0;
            ++blockCnt;
        }
    }
    if (entryInBlockCnt > 0) {
        string compressed;
        compressed = block;
        blockSeg += compressed;
        oris.push_back(ori);
        cmps.push_back(cmp);
        ori += block.size();
        cmp += compressed.size();
        block.clear();
        ++blockCnt;
    }
    keys.push_back(0);
    offsets.push_back(offset);
    seqNums.push_back(0); // ??
    oris.push_back(ori);
    cmps.push_back(cmp);
    save(blockSeg);
}

SSTable::SSTable(const vector<Entry> &entries, size_t &pos, const SSTableId &id, TableCache *tableCache)
        : id(id), tableCache(tableCache) {
    size_t n = entries.size();
    entryCnt = 0;
    blockCnt = 0;
    uint64_t offset = 0;
    uint64_t ori = 0;
    uint64_t cmp = 0;
    string block;
    block.reserve(Option::BLOCK_SPACE);
    string blockSeg;
    blockSeg.reserve(Option::SST_SPACE);
    uint64_t entryInBlockCnt = 0;
    while (pos < n) {
        Entry entry = entries[pos++];
        keys.push_back(entry.key);
        offsets.push_back(offset);
        seqNums.push_back(entry.seqNum);
        offset += entry.value.size();
        ++entryCnt;
        block += entry.value;
        ++entryInBlockCnt;
        if (block.size() >= Option::BLOCK_SPACE) {
            string compressed;
            compressed = block;
            blockSeg += compressed;
            oris.push_back(ori);
            cmps.push_back(cmp);
            ori += block.size();
            cmp += compressed.size();
            block.clear();
            entryInBlockCnt = 0;
            ++blockCnt;
            if (indexSpace() + cmp >= Option::SST_SPACE)
                break;
        }
    }
    if (entryInBlockCnt > 0) {
        string compressed;
        compressed = block;
        blockSeg += compressed;
        oris.push_back(ori);
        cmps.push_back(cmp);
        ori += block.size();
        cmp += compressed.size();
        block.clear();
        ++blockCnt;
    }
    keys.push_back(0);
    offsets.push_back(offset);
    seqNums.push_back(0); // ??
    oris.push_back(ori);
    cmps.push_back(cmp);
    save(blockSeg);
}

SearchResult SSTable::search(uint64_t key) const {
    uint64_t left = 0;
    uint64_t right = entryCnt;
    while (right - left > 2) {
        uint64_t mid = left + (right - left) / 2;
        if (keys[mid] < key)
            left = mid;
        else if (keys[mid] > key)
            right = mid;
        else
            return {true, locate(mid)};
    }
    if (keys[left] == key)
        return {true, locate(left)};
    else if (keys[right - 1] == key)
        return {true, locate(right - 1)};
    else
        return false;
}

vector<Entry> SSTable::load() const {
    vector<Entry> entries;
    uint64_t k = 0;
    string block = loadBlock(0);
    for (uint64_t i = 0; i < entryCnt; ++i) {
        if (offsets[i + 1] > oris[k + 1])
            block = loadBlock(++k);
        uint64_t key = keys[i];
        string value = block.substr(offsets[i] - oris[k], offsets[i + 1] - offsets[i]);
        uint64_t seqNum = seqNums[i];
        entries.emplace_back(key, value, seqNum);
    }
    return entries;
}

void SSTable::remove() const {
    if (Option::TABLE_CACHE)
        tableCache->close(id);
    filesystem::remove(filesystem::path(id.name()));
}

uint64_t SSTable::number() const {
    return id.no;
}

uint64_t SSTable::lower() const {
    return keys[0];
}

uint64_t SSTable::upper() const {
    return keys[entryCnt - 1];
}

uint64_t SSTable::space() const {
    return indexSpace() + blockSpace();
}

void SSTable::save(const string &blockSeg) {
    ofstream ofs(id.name(), ios::binary);
    ofs.write((char *) &entryCnt, sizeof(uint64_t));
    for (uint64_t i = 0; i <= entryCnt; ++i) {
        ofs.write((char *) &keys[i], sizeof(uint64_t));
        ofs.write((char *) &offsets[i], sizeof(uint64_t));
        ofs.write((char *) &seqNums[i], sizeof(uint64_t));
    }
    ofs.write((char *) &blockCnt, sizeof(uint64_t));
    for (uint64_t i = 0; i <= blockCnt; ++i) {
        ofs.write((char *) &oris[i], sizeof(uint64_t));
        ofs.write((char *) &cmps[i], sizeof(uint64_t));
    }
    ofs.write(blockSeg.data(), blockSpace());
    ofs.close();
}

Location SSTable::locate(uint64_t pos) const {
    uint64_t k = 0;
    while (offsets[pos + 1] > oris[k + 1])
        ++k;
    return {this, k, offsets[pos] - oris[k], offsets[pos + 1] - offsets[pos]};
}

string SSTable::loadBlock(uint64_t pos) const {
    string block;
    char *buf = new char[cmps[pos + 1] - cmps[pos]];
    if (Option::TABLE_CACHE) {
        ifstream *ifs = tableCache->open(id);
        ifs->seekg(indexSpace() + cmps[pos], ios::beg);
        ifs->read(buf, cmps[pos + 1] - cmps[pos]);
    } else {
        ifstream ifs(id.name(), ios::binary);
        ifs.seekg(indexSpace() + cmps[pos], ios::beg);
        ifs.read(buf, cmps[pos + 1] - cmps[pos]);
        ifs.close();
    }
    block.assign(buf, cmps[pos + 1] - cmps[pos]);
    delete[] buf;
    return block;
}

uint64_t SSTable::indexSpace() const {
    return (entryCnt * 3 + blockCnt * 2 + 7) * sizeof(uint64_t);
}

uint64_t SSTable::blockSpace() const {
    return cmps.back();
}
