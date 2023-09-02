#include "../../../include/disk/sstable/SSTable.h"

#include <fstream>
#include <filesystem>
#include <iostream>
#include <utility>

SSTable::SSTable(SSTableId id)
        : id(std::move(id)) {
    //TODO BlockCnt 초기화 필요?
    SSTableDataLocation loc = loadAll();
    min = loc.keys[0];
    max = loc.keys[entryCnt - 1];
    for (uint64_t i = 0; i <= entryCnt; i++) {
        bloomfilter.insert(loc.keys[i]);
    }
    setSeqNumFilter(loc.seqNums);
    size = loc.cmps.back();
}

SSTable::SSTable(const SkipList &mem, SSTableId id)
        : id(std::move(id)) {
    // 기존 skiplist 블룸필터
    bloomfilter = mem.bloomfilter;

    vector<uint64_t> keys;
    vector<uint64_t> offsets;
    vector<uint64_t> seqNums;
    vector<uint64_t> oris;
    vector<uint64_t> cmps;

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

    setSeqNumFilter(seqNums);

    keys.push_back(0);
    offsets.push_back(offset);
    seqNums.push_back(0);
    oris.push_back(ori);
    cmps.push_back(cmp);

    min = keys[0];
    max = keys[entryCnt - 1];
    size = cmps.back();

    save(keys, offsets, seqNums, oris, cmps, blockSeg);
}

SSTable::SSTable(const std::vector<Entry> &entries, size_t &pos, const SSTableId &id)
        : id(id) {
    vector<uint64_t> keys;
    vector<uint64_t> offsets;
    vector<uint64_t> seqNums;
    vector<uint64_t> oris;
    vector<uint64_t> cmps;

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

        // 블룸필터에 추가
        bloomfilter.insert(entry.key);

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

    setSeqNumFilter(seqNums);

    keys.push_back(0);
    //여긴 마지막에 seqNums.push_back(0) 안해줘도 되나?
    offsets.push_back(offset);
    oris.push_back(ori);
    cmps.push_back(cmp);

    min = keys[0];
    max = keys[entryCnt - 1];
    size = cmps.back();

    save(keys, offsets, seqNums, oris, cmps, blockSeg);
}

SearchResult SSTable::search(uint64_t key, uint64_t seqNum) const {
    // 현재는 무조건 Disk I/O
    // 먼저 SkipList의 블룸 필터를 이용하여 key가 존재하지 않는지 확인
    if (!bloomfilter.hasKey(key)) {
        return false; // 블룸 필터에 없으면 키가 없다고 판단
    }

    if(!seqNumFilter.isVisible(seqNum)){
        return false;
    }
    SSTableDataLocation loc = loadAll();

    uint64_t left = 0;
    uint64_t right = entryCnt;
    uint64_t mid;
    while (right - left > 2) {
        mid = left + (right - left) / 2;
        if (loc.keys[mid] == key) {
            return filterBySeqNum(key, seqNum, loc, mid);
        }
        if (loc.keys[mid] < key) {
            left = mid;
            continue;
        }
        if (loc.keys[mid] > key) {
            right = mid;
        }
    }
    if (loc.keys[left] == key) {
        return filterBySeqNum(key, seqNum, loc, left);
    }
    if (loc.keys[right - 1] == key) {
        return filterBySeqNum(key, seqNum, loc, right - 1);
    }
    return false;
}

SearchResult SSTable::filterBySeqNum(uint64_t target_key, uint64_t target_seqNum,
                                     SSTableDataLocation loc, uint64_t pos) const {
    uint64_t seqNum = loc.seqNums[pos];
    if (seqNum < target_seqNum) {
        while (loc.keys[pos] == target_key && seqNum <= target_seqNum) {
            if (pos == entryCnt) return false;
            seqNum = loc.seqNums[++pos];
        }
        --pos;
    } else if (target_seqNum < seqNum) {
        while (loc.keys[pos] == target_key && target_seqNum <= seqNum) {
            if (pos == -1) return false;
            seqNum = loc.seqNums[--pos];
        }
        ++pos;
    }
    if (loc.keys[pos] != target_key || target_seqNum < loc.seqNums[pos]) {
        return false;
    }
    Location location = locate(loc, pos);
    string value = loadBlock(loc.cmps, location.pos).substr(location.offset, location.len);
    return {true, value};
}

SSTableDataLocation SSTable::loadAll() const {
    vector<uint64_t> keys;
    vector<uint64_t> offsets;
    vector<uint64_t> seqNums;
    vector<uint64_t> oris;
    vector<uint64_t> cmps;

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

    return {keys, offsets, seqNums, oris, cmps};
}

vector<Entry> SSTable::load() const {
    SSTableDataLocation loc = loadAll();
    vector<Entry> entries;
    uint64_t k = 0;
    string block = loadBlock(loc.cmps, 0);
    for (uint64_t i = 0; i < entryCnt; ++i) {
        if (loc.offsets[i + 1] > loc.oris[k + 1]) {
            block = loadBlock(loc.cmps, ++k);
        }
        uint64_t key = loc.keys[i];
        string value = block.substr(loc.offsets[i] - loc.oris[k], loc.offsets[i + 1] - loc.offsets[i]);
        uint64_t seqNum = loc.seqNums[i];
        entries.emplace_back(key, value, seqNum);
    }
    return entries;
}

void SSTable::remove() const {
    filesystem::remove(filesystem::path(id.name()));
}

uint64_t SSTable::number() const {
    return id.no;
}

uint64_t SSTable::lower() const {
    return min;
}

uint64_t SSTable::upper() const {
    return max;
}

uint64_t SSTable::space() const {
    return indexSpace() + blockSpace();
}

void SSTable::save(vector<uint64_t> keys, vector<uint64_t> offsets, vector<uint64_t> seqNums,
                   vector<uint64_t> oris, vector<uint64_t> cmps, const string &blockSeg) {
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

Location SSTable::locate(SSTableDataLocation loc, uint64_t pos) const {
    uint64_t k = 0;
    while (loc.offsets[pos + 1] > loc.oris[k + 1])
        ++k;
    // sst pos offset len
    return {this, k, loc.offsets[pos] - loc.oris[k], loc.offsets[pos + 1] - loc.offsets[pos]};
}

string SSTable::loadBlock(vector<uint64_t> cmps, uint64_t pos) const {
    string block;
    char *buf = new char[cmps[pos + 1] - cmps[pos]];

    ifstream ifs(id.name(), ios::binary);
    ifs.seekg(indexSpace() + cmps[pos], ios::beg);
    ifs.read(buf, cmps[pos + 1] - cmps[pos]);
    ifs.close();

    block.assign(buf, cmps[pos + 1] - cmps[pos]);
    delete[] buf;
    return block;
}

uint64_t SSTable::indexSpace() const {
    return (entryCnt * 3 + blockCnt * 2 + 7) * sizeof(uint64_t);
}

uint64_t SSTable::blockSpace() const {
    return size;
}

void SSTable::print(uint64_t i) const {
    cout << "--- SSTable " << i << " ---" << endl;
    vector<Entry> entries = load();
    for (const auto &entry: entries) {
        cout << "key: " << entry.key << ", value: " << entry.value << ", seqNum: " << entry.seqNum << endl;
    }
    cout << "\n" << endl;
}

void SSTable::setSeqNumFilter(const vector<uint64_t>& seqNums)
{
    for (size_t i = 0; i <= seqNums.size(); ++i) {
        if (seqNums[i] < seqNumFilter.minSeqNum) {
            seqNumFilter.minSeqNum = seqNums[i];
        }
        if (seqNums[i] > seqNumFilter.maxSeqNum) {
            seqNumFilter.maxSeqNum = seqNums[i];
        }
    }
}

