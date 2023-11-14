#include "../../../include/disk/sstable/SSTable2.h"

#include <fstream>
#include <filesystem>
#include <iostream>
#include <utility>
#include <algorithm>

SSTable2::SSTable2(SSTableId id) : id(std::move(id)) {
//    ifstream in(id.name(), ios::binary);
//
    // TODO : 파일 읽어들여서 loadAll 만들어야함?
    cout << "1번 생성자" << endl;
}

SSTable2::SSTable2(const SkipList &mem, SSTableId id) : id(std::move(id)) {
    bloomfilter = mem.bloomFilter;
    seqNumFilter = mem.seqNumFilter;

    entryCnt = mem.size();
    blockCnt = 0;

    minKey = UINT64_MAX;
    maxKey = 0;
    minSeqNum = UINT64_MAX;

    vector<vector<uint64_t>> keys;
    vector<vector<uint64_t>> seqNums;
    vector<vector<uint64_t>> valueSizes;
    vector<vector<string>> values;

    vector<uint64_t> block_keys;
    vector<uint64_t> block_seqNums;
    vector<uint64_t> block_valueSizes;
    vector<string> block_values;

    vector<uint64_t> minKeys;
    vector<uint64_t> dataBlockOffsets;

    uint64_t blockSize = 0;
    uint64_t dataBlockOffset = 0;
    uint64_t entryInBlockCnt = 0;

    SkipList::Iterator itr = mem.iterator();

    while (itr.hasNext()) {
        Entry entry = itr.next();

        uint64_t seqNum = entry.seqNum;
        uint64_t key = entry.key;
        string value = entry.value;

        // key, seqNum, valueSize, value
        block_keys.push_back(key);
        block_seqNums.push_back(seqNum);
        block_valueSizes.push_back(entry.value.size());
        block_values.push_back(value);

        blockSize += 3 * sizeof(uint64_t);
        // value
        blockSize += entry.value.size();

        ++entryInBlockCnt;

        if (blockSize >= Option::BLOCK_SPACE) {
            // entryCnt
            blockSize += sizeof(uint64_t);

            dataBlockOffsets.push_back(dataBlockOffset);
            minKeys.push_back(block_keys[0]);
            keys.push_back(block_keys);
            seqNums.push_back(block_seqNums);
            valueSizes.push_back(block_valueSizes);
            values.push_back(block_values);

            dataBlockOffset += blockSize;

            block_keys.clear();
            block_seqNums.clear();
            block_valueSizes.clear();
            block_values.clear();

            blockCnt++;
            blockSize = 0;
            entryInBlockCnt = 0;
        }
    }

    if (entryInBlockCnt != 0) {
        blockSize += sizeof(uint64_t);

        dataBlockOffsets.push_back(dataBlockOffset);
        minKeys.push_back(block_keys[0]);
        keys.push_back(block_keys);
        seqNums.push_back(block_seqNums);
        valueSizes.push_back(block_valueSizes);
        values.push_back(block_values);

        dataBlockOffset += blockSize;
        blockCnt++;
    }

    minKey = keys[0][0];
    maxKey = keys[blockCnt - 1][keys[blockCnt - 1].size() - 1];
    setMinSeqNum(seqNums);
    seqNumFilter.minSeqNum = minSeqNum;
    indexBlockAddress = dataBlockOffset;
    save(keys, seqNums, valueSizes, values, minKeys, dataBlockOffsets);
}

SSTable2::SSTable2(const std::vector<Entry> &entries, size_t &pos, const SSTableId &id) : id(id) {
    blockCnt = 0;
    minKey = UINT64_MAX;
    maxKey = 0;
    minSeqNum = UINT64_MAX;

    vector<vector<uint64_t>> keys;
    vector<vector<uint64_t>> seqNums;
    vector<vector<uint64_t>> valueSizes;
    vector<vector<string>> values;

    vector<uint64_t> block_keys;
    vector<uint64_t> block_seqNums;
    vector<uint64_t> block_valueSizes;
    vector<string> block_values;

    vector<uint64_t> minKeys;
    vector<uint64_t> dataBlockOffsets;

    uint64_t blockSize = 0;
    uint64_t dataBlockOffset = 0;
    uint64_t entryInBlockCnt = 0;

    size_t n = entries.size();
    while (pos < n) {
        Entry entry = entries[pos++];

        uint64_t key = entry.key;
        uint64_t seqNum = entry.seqNum;
        string value = entry.value;

        bloomfilter.insert(key);
        minSeqNum = min(minSeqNum, seqNum);

        // key, seqNum, valueSize, value
        block_keys.push_back(key);
        block_seqNums.push_back(seqNum);
        block_valueSizes.push_back(entry.value.size());
        block_values.push_back(value);

        blockSize += 3 * sizeof(uint64_t);
        // value
        blockSize += entry.value.size();

        ++entryInBlockCnt;

        if (blockSize >= Option::BLOCK_SPACE) {
            // entryCnt
            blockSize += sizeof(uint64_t);

            dataBlockOffsets.push_back(dataBlockOffset);
            minKeys.push_back(block_keys[0]);
            keys.push_back(block_keys);
            seqNums.push_back(block_seqNums);
            valueSizes.push_back(block_valueSizes);
            values.push_back(block_values);

            dataBlockOffset += blockSize;

            block_keys.clear();
            block_seqNums.clear();
            block_valueSizes.clear();
            block_values.clear();

            blockCnt++;
            blockSize = 0;
            entryInBlockCnt = 0;
        }
    }

    if (entryInBlockCnt != 0) {
        blockSize += sizeof(uint64_t);

        dataBlockOffsets.push_back(dataBlockOffset);
        minKeys.push_back(block_keys[0]);
        keys.push_back(block_keys);
        seqNums.push_back(block_seqNums);
        valueSizes.push_back(block_valueSizes);
        values.push_back(block_values);

        dataBlockOffset += blockSize;
        blockCnt++;
    }

    minKey = keys[0][0];
    maxKey = keys[blockCnt - 1][keys[blockCnt - 1].size() - 1];
    seqNumFilter.minSeqNum = minSeqNum;
    setMinSeqNum(seqNums);
    indexBlockAddress = dataBlockOffset;
    save(keys, seqNums, valueSizes, values, minKeys, dataBlockOffsets);
}

void SSTable2::save(vector<vector<uint64_t>> keys,
                    vector<vector<uint64_t>> seqNums,
                    vector<vector<uint64_t>> valueSizes,
                    vector<vector<string>> values,
                    vector<uint64_t> minKeys,
                    vector<uint64_t> dataBlockOffsets) {
    ofstream out(id.name(), ios::binary);

    for (uint64_t i = 0; i < blockCnt; i++) {
        // entryCnt
        uint64_t keySize = keys[i].size();
        out.write((char *) &keySize, sizeof(uint64_t));
        for (int j = 0; j < keys[i].size(); ++j) {
            out.write((char *) &keys[i][j], sizeof(uint64_t));
            out.write((char *) &seqNums[i][j], sizeof(uint64_t));
            out.write((char *) &valueSizes[i][j], sizeof(uint64_t));
            out.write(values[i][j].data(), valueSizes[i][j]);
        }
    }
    for (uint64_t i = 0; i < blockCnt; i++) {
        out.write((char *) &minKeys[i], sizeof(uint64_t));
        out.write((char *) &dataBlockOffsets[i], sizeof(uint64_t));
    }
    out.close();
}

SearchResult SSTable2::search(uint64_t key, uint64_t seqNum) const {
    if (!bloomfilter.hasKey(key)) {
        return false;
    }
    if (!seqNumFilter.isVisible(seqNum)) {
        return false;
    }
    DataBlockLocation loc = loadDataBlockLocation();
    if (loc.minKeys.empty()) {
        return false;
    }

    uint64_t blockId = 0;
    uint64_t i = 0;
    while (true) {
        if (i == loc.minKeys.size()) {
            blockId = i - 1;
            break;
        }
        if (loc.minKeys[i] <= key && key < loc.minKeys[i + 1]) {
            blockId = i;
            break;
        }
        i++;
    }

    BlockLocation blockLocation = loadBlockLocation(loc.dataBlockOffsets[blockId]);
    uint64_t left = 0;
    uint64_t right = blockLocation.keys.size();
    uint64_t mid = 0;
    while (right - left > 2) {
        mid = left + (right - left) / 2;
        if (blockLocation.keys[mid] == key) {
            return filterBySeqNum(key, seqNum, blockLocation, mid);
        }
        if (blockLocation.keys[mid] < key) {
            left = mid;
            continue;
        }
        if (blockLocation.keys[mid] > key) {
            right = mid;
        }
    }
    if (blockLocation.keys[left] == key) {
        return filterBySeqNum(key, seqNum, blockLocation, left);
    }
    if (blockLocation.keys[right - 1] == key) {
        return filterBySeqNum(key, seqNum, blockLocation, right - 1);
    }
    return false;
}

SearchResult SSTable2::filterBySeqNum(uint64_t target_key, uint64_t target_seqNum,
                                      BlockLocation loc, uint64_t pos) const {
    uint64_t seqNum = loc.seqNums[pos];

    if (loc.keys[pos] != target_key || target_seqNum < loc.seqNums[pos]) {
        return false;
    }

    if (seqNum < target_seqNum) {
        while (loc.keys[pos] == target_key && seqNum <= target_seqNum) {
            if (pos == loc.keys.size()) return false;
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

    string value = loadValue(loc, pos);
    return {true, value};
}

string SSTable2::loadValue(BlockLocation loc, uint64_t pos) const {
    string value;
    uint64_t k = sizeof(uint64_t);
    k += 3 * sizeof(uint64_t) * pos;

    for (uint64_t i = 0; i < pos; ++i) {
        k += loc.valueSizes[i];
    }

    ifstream in(id.name(), ios::binary);
    in.seekg(k + 3 * sizeof(uint64_t), ios::beg);

    char *buf = new char[loc.valueSizes[pos]];
    in.read(buf, loc.valueSizes[pos]);
    in.close();

    value.assign(buf, loc.valueSizes[pos]);
    return value;
}

BlockLocation SSTable2::loadBlockLocation(uint64_t pos) const {
    ifstream in(id.name(), ios::binary);
    in.seekg(pos, ios::beg);

    uint64_t entryCnt;
    in.read((char *) &entryCnt, sizeof(uint64_t));

    vector<uint64_t> keys;
    vector<uint64_t> seqNums;
    vector<uint64_t> valueSizes;

    for (uint64_t i = 0; i < entryCnt; i++) {
        uint64_t key;
        uint64_t seqNum;
        uint64_t valueSize;
        in.read((char *) &key, sizeof(uint64_t));
        in.read((char *) &seqNum, sizeof(uint64_t));
        in.read((char *) &valueSize, sizeof(uint64_t));
        keys.push_back(key);
        seqNums.push_back(seqNum);
        valueSizes.push_back(valueSize);
        in.seekg(valueSize, ios::cur);
    }
    in.close();

    return BlockLocation(keys, seqNums, valueSizes);
}


vector<Entry> SSTable2::load() const {
    DataBlockLocation loc = loadDataBlockLocation();
    vector<Entry> entries;

    ifstream in(id.name(), ios::binary);
    uint64_t entryCnt;

    for (uint64_t i = 0; i < loc.dataBlockOffsets.size(); ++i) {
        in.seekg(loc.dataBlockOffsets[i], ios::beg);
        in.read((char *) &entryCnt, sizeof(uint64_t));
        for (uint64_t j = 0; j < entryCnt; j++) {
            uint64_t key;
            uint64_t seqNum;
            uint64_t valueSize;
            string value;
            in.read((char *) &key, sizeof(uint64_t));
            in.read((char *) &seqNum, sizeof(uint64_t));
            in.read((char *) &valueSize, sizeof(uint64_t));
            char *buf = new char[valueSize];
            in.read(buf, valueSize);
            value.assign(buf, valueSize);
            entries.emplace_back(key, value, seqNum);
        }
    }
    in.close();

    return entries;
}


DataBlockLocation SSTable2::loadDataBlockLocation() const {
    ifstream in(id.name(), ios::binary);
    in.seekg(indexBlockAddress, ios::beg);

    vector<uint64_t> minKeys;
    vector<uint64_t> dataBlockOffsets;

    for (uint64_t i = 0; i < blockCnt; i++) {
        uint64_t minKey;
        uint64_t dataBlockOffset;
        in.read((char *) &minKey, sizeof(uint64_t));
        in.read((char *) &dataBlockOffset, sizeof(uint64_t));
        minKeys.push_back(minKey);
        dataBlockOffsets.push_back(dataBlockOffset);
    }
    in.close();

    return DataBlockLocation(minKeys, dataBlockOffsets);
}

uint64_t SSTable2::indexSpace() const {
    return (blockCnt * 2) * sizeof(uint64_t);
}

uint64_t SSTable2::blockSpace() const {
    return indexBlockAddress;
}

void SSTable2::remove() const {
    filesystem::remove(filesystem::path(id.name()));
}

uint64_t SSTable2::getId() const {
    return id.id;
}

uint64_t SSTable2::getMinKey() const {
    return minKey;
}

uint64_t SSTable2::getMaxKey() const {
    return maxKey;
}

uint64_t SSTable2::getMinSeqNum() const {
    return minSeqNum;
}

uint64_t SSTable2::space() const {
    return indexSpace() + blockSpace();
}

void SSTable2::print(uint64_t i) const {
    cout << "--- SSTable " << i << " ---" << endl;
    cout << "<<" << id.name() << ">>" << endl;
    vector<Entry> entries = load();
    for (const auto &entry: entries) {
        cout << "key: " << entry.key << ", value: " << entry.value << ", seqNum: " << entry.seqNum << endl;
    }
    cout << "\n" << endl;
}

void SSTable2::setMinSeqNum(const vector<vector<uint64_t>> seqNums) {
    for (const std::vector<uint64_t> &innerVector: seqNums) {
        for (uint64_t value: innerVector) {
            if (value < minSeqNum) {
                minSeqNum = value;
            }
        }
    }
}

// TODO : LoadAll, setSeqNumFilter
