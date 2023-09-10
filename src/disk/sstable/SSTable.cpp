#include "../../../include/disk/sstable/SSTable.h"

#include <fstream>
#include <filesystem>
#include <iostream>
#include <utility>

SSTable::SSTable(SSTableId id) : id(std::move(id)) {
    // TODO: BlockCnt 초기화 필요?
    SSTableDataLocation loc = loadAll();
    minKey = loc.keys[0];
    maxKey = loc.keys[entryCnt - 1];
    for (uint64_t i = 0; i <= entryCnt; i++) {
        bloomfilter.insert(loc.keys[i]);
    }
    size = loc.dataBlockOffsets.back();
}

SSTable::SSTable(const SkipList &mem, SSTableId id) : id(std::move(id)) {
    bloomfilter = mem.bloomFilter;
    entryCnt = mem.size();
    blockCnt = 0;

    vector<uint64_t> keys;
    vector<uint64_t> offsets;
    vector<uint64_t> seqNums;
    vector<uint64_t> dataBlockOffsets;

    uint64_t offset = 0;
    uint64_t dataBlockOffset = 0;
    uint64_t entryInBlockCnt = 0;

    string block;
    block.reserve(Option::BLOCK_SPACE);
    string blockSegment;
    blockSegment.reserve(Option::SST_SPACE);

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
            blockSegment += block;
            dataBlockOffsets.push_back(dataBlockOffset);
            dataBlockOffset += block.size();
            block.clear();
            entryInBlockCnt = 0;
            ++blockCnt;
        }
    }
    if (entryInBlockCnt > 0) {
        blockSegment += block;
        dataBlockOffsets.push_back(dataBlockOffset);
        dataBlockOffset += block.size();
        block.clear();
        ++blockCnt;
    }

    keys.push_back(0);
    offsets.push_back(offset);
    seqNums.push_back(0);
    dataBlockOffsets.push_back(dataBlockOffset);

    minKey = keys[0];
    maxKey = keys[entryCnt - 1];
    size = dataBlockOffsets.back();

    save(keys, offsets, seqNums, dataBlockOffsets, blockSegment);
}

SSTable::SSTable(const std::vector<Entry> &entries, size_t &pos, const SSTableId &id) : id(id) {
    vector<uint64_t> keys;
    vector<uint64_t> offsets;
    vector<uint64_t> seqNums;
    vector<uint64_t> dataBlockOffsets;

    size_t n = entries.size();
    entryCnt = 0;
    blockCnt = 0;
    uint64_t offset = 0;
    uint64_t dataBlockOffset = 0;

    string block;
    block.reserve(Option::BLOCK_SPACE);
    string blockSegment;
    blockSegment.reserve(Option::SST_SPACE);
    uint64_t entryInBlockCnt = 0;

    while (pos < n) {
        Entry entry = entries[pos++];
        keys.push_back(entry.key);
        bloomfilter.insert(entry.key);

        offsets.push_back(offset);
        seqNums.push_back(entry.seqNum);

        offset += entry.value.size();
        ++entryCnt;
        block += entry.value;
        ++entryInBlockCnt;

        if (block.size() >= Option::BLOCK_SPACE) {
            blockSegment += block;
            dataBlockOffsets.push_back(dataBlockOffset);
            dataBlockOffset += block.size();
            block.clear();
            entryInBlockCnt = 0;
            ++blockCnt;
            if (indexSpace() + dataBlockOffset >= Option::SST_SPACE)
                break;
        }
    }
    if (entryInBlockCnt > 0) {
        string compressed;
        compressed = block;
        blockSegment += compressed;
        dataBlockOffsets.push_back(dataBlockOffset);
        dataBlockOffset += block.size();
        block.clear();
        ++blockCnt;
    }

    keys.push_back(0);
    seqNums.push_back(0);
    offsets.push_back(offset);
    dataBlockOffsets.push_back(dataBlockOffset);

    minKey = keys[0];
    maxKey = keys[entryCnt - 1];
    size = dataBlockOffsets.back();

    save(keys, offsets, seqNums, dataBlockOffsets, blockSegment);
}

SearchResult SSTable::search(uint64_t key, uint64_t seqNum) const {
    // 현재는 무조건 Disk I/O

    if (!bloomfilter.hasKey(key)) {
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
    string value = loadValue(loc, pos);
    return {true, value};
}

SSTableDataLocation SSTable::loadAll() const {
    vector<uint64_t> keys;
    vector<uint64_t> offsets;
    vector<uint64_t> seqNums;
    vector<uint64_t> dataBlockOffsets;

    ifstream ifs(id.name(), ios::binary);

    // EntryIndex
    ifs.read((char *) &entryCnt, sizeof(uint64_t)); // n
    for (uint64_t i = 0; i <= entryCnt; i++) {
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
    for (uint64_t i = 0; i <= blockCnt; i++) {
        uint64_t dataBlockOffset;
        ifs.read((char *) &dataBlockOffset, sizeof(uint64_t));
        dataBlockOffsets.push_back(dataBlockOffset);
    }
    ifs.close();

    return {keys, offsets, seqNums, dataBlockOffsets};
}

vector<Entry> SSTable::load() const {
    SSTableDataLocation loc = loadAll();
    vector<Entry> entries;

    uint64_t pos = 0;
    string block = loadBlock(loc.dataBlockOffsets, pos);
    for (uint64_t i = 0; i < entryCnt; i++) {
        if (loc.offsets[i + 1] > loc.dataBlockOffsets[pos + 1]) {
            block = loadBlock(loc.dataBlockOffsets, pos++);
        }
        uint64_t key = loc.keys[i];
        string value = block.substr(loc.offsets[i] - loc.dataBlockOffsets[pos],
                                    loc.offsets[i + 1] - loc.offsets[i]);
        uint64_t seqNum = loc.seqNums[i];
        entries.emplace_back(key, value, seqNum);
    }
    return entries;
}

void SSTable::remove() const {
    filesystem::remove(filesystem::path(id.name()));
}

uint64_t SSTable::getId() const {
    return id.id;
}

uint64_t SSTable::getMinKey() const {
    return minKey;
}

uint64_t SSTable::getMaxKey() const {
    return maxKey;
}

uint64_t SSTable::space() const {
    return indexSpace() + blockSpace();
}

void SSTable::save(vector<uint64_t> keys, vector<uint64_t> offsets, vector<uint64_t> seqNums,
                   vector<uint64_t> dataBlockOffsets, const string &blockSegment) {
    ofstream ofs(id.name(), ios::binary);
    ofs.write((char *) &entryCnt, sizeof(uint64_t));
    for (uint64_t i = 0; i <= entryCnt; i++) {
        ofs.write((char *) &keys[i], sizeof(uint64_t));
        ofs.write((char *) &offsets[i], sizeof(uint64_t));
        ofs.write((char *) &seqNums[i], sizeof(uint64_t));
    }
    ofs.write((char *) &blockCnt, sizeof(uint64_t));
    for (uint64_t i = 0; i <= blockCnt; i++) {
        ofs.write((char *) &dataBlockOffsets[i], sizeof(uint64_t));
    }
    ofs.write(blockSegment.data(), blockSpace());
    ofs.close();
}

string SSTable::loadValue(SSTableDataLocation loc, uint64_t pos) const {
    uint64_t k = 0;
    while (loc.offsets[pos + 1] > loc.dataBlockOffsets[k + 1]) {
        k++;
    }
    uint64_t offset = loc.offsets[pos] - loc.dataBlockOffsets[k];
    uint64_t len = loc.offsets[pos + 1] - loc.offsets[pos];

    return loadBlock(loc.dataBlockOffsets, k).substr(offset, len);
}

string SSTable::loadBlock(vector<uint64_t> dataBlockOffsets, uint64_t pos) const {
    string block;
    char *buf = new char[dataBlockOffsets[pos + 1] - dataBlockOffsets[pos]];

    ifstream ifs(id.name(), ios::binary);
    ifs.seekg(indexSpace() + dataBlockOffsets[pos], ios::beg);
    ifs.read(buf, dataBlockOffsets[pos + 1] - dataBlockOffsets[pos]);
    ifs.close();

    block.assign(buf, dataBlockOffsets[pos + 1] - dataBlockOffsets[pos]);
    delete[] buf;
    return block;
}

uint64_t SSTable::indexSpace() const {
    return (entryCnt * 3 + blockCnt * 1 + 6) * sizeof(uint64_t);
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
