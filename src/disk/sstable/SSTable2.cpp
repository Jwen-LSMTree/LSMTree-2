//
// Created by 김 정우 on 10/28/23.
//

#include "../../../include/disk/sstable/SSTable2.h"

#include <fstream>
#include <filesystem>
#include <iostream>
#include <utility>
#include <algorithm>

SSTable2::SSTable2(SSTableId id) : id(std::move(id)) {
    ifstream in(id.name(), ios::binary);

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
    uint64_t blockMinKey = UINT64_MAX;

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

        blockMinKey = min(blockMinKey, key);

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
//    minSeqNum = *min_element(seqNums.begin(), seqNums.end());
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
        out.write((char *) keys[i].size(), sizeof(uint64_t));
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

    ifstream in(id.name(), ios::binary);

    uint64_t minKey;
    in.read((char *) &minKey, sizeof(uint64_t));
    cout << minKey << endl;

    uint64_t Key;
    in.read((char *) &Key, sizeof(uint64_t));
    cout << Key << endl;

    in.close();
}

SearchResult SSTable2::search(uint64_t key, uint64_t seqNum) const {
    if (!bloomfilter.hasKey(key)) {
        return false;
    }
    if(!seqNumFilter.isVisible(seqNum)){
        return false;
    }
    DataBlockLocation loc = loadDataBlockLocation();

    uint64_t left = 0;
    uint64_t right = blockCnt;
    uint64_t mid = 0;
    while (right - left > 2) {
        mid = left + (right - left) / 2;
        if (loc.minKeys[mid] == key) {
            break;
        }
        if (loc.minKeys[mid] < key) {
            left = mid;
            continue;
        }
        if (loc.minKeys[mid] > key) {
            right = mid;
        }
    }
    // mid가 찾을 위치
    BlockLocation blockLocation = loadBlockLocation(loc.dataBlockOffsets[mid]);

    left = 0;
    right = blockLocation.keys.size();
    while (right - left > 2) {
        mid = left + (right - left) / 2;
        if (blockLocation.keys[mid] == key) {
            filterBySeqNum(key, seqNum, blockLocation, mid);
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
    uint64_t k = 0;

    k += 3 * sizeof(uint64_t) * pos;

    for (uint64_t i = 0; i < pos; ++i) {
        k += loc.valueSizes[i];
    }

    ifstream in(id.name(), ios::binary);
    in.seekg(k + 3 * sizeof(uint64_t), ios::beg);

    char* buf = new char[loc.valueSizes[pos]];
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
    }

    in.close();

    return BlockLocation(keys, seqNums, valueSizes);
}



DataBlockLocation SSTable2::loadDataBlockLocation() const {
    ifstream in(id.name(), ios::binary);

    uint64_t minKey;
    in.read((char *) &minKey, sizeof(uint64_t));
    cout << minKey << endl;

    uint64_t Key;
    in.read((char *) &Key, sizeof(uint64_t));
    cout << Key << endl;

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
