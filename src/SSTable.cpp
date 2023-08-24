#include "SSTable.h"
#include "Util.h"
#include "Option.h"
// #include "snappy.h"
#include <fstream>
#include <filesystem>
#include <iostream>

SSTable::SSTable(const SSTableId &id, TableCache *tableCache)
    : id(id), tableCache(tableCache)
{
    SSTableDataLocation loc = loadAll();
    min = loc.keys[0];
    max = loc.keys[entryCnt-1];
    size = loc.cmps.back();
}

SSTable::SSTable(const SkipList &mem, const SSTableId &id, TableCache *tableCache)
    : id(id), tableCache(tableCache)
{
    std::vector<uint64_t> keys;
    std::vector<uint64_t> offsets;
    std::vector<uint64_t> oris;
    std::vector<uint64_t> cmps;
    entryCnt = mem.size();
    blockCnt = 0;
    uint64_t offset = 0;
    uint64_t ori = 0;
    uint64_t cmp = 0;
    std::string block;
    block.reserve(Option::BLOCK_SPACE);
    std::string blockSeg;
    blockSeg.reserve(Option::SST_SPACE);
    uint64_t entryInBlockCnt = 0;
    SkipList::Iterator itr = mem.iterator();
    while (itr.hasNext())
    {
        Entry entry = itr.next();
        keys.push_back(entry.key);
        offsets.push_back(offset);
        offset += entry.value.size();
        block += entry.value;
        ++entryInBlockCnt;
        if (block.size() >= Option::BLOCK_SPACE)
        {
            std::string compressed;

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
    if (entryInBlockCnt > 0)
    {
        std::string compressed;
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
    oris.push_back(ori);
    cmps.push_back(cmp);
    min = keys[0];
    max = keys[entryCnt-1];
    size = cmps.back();
    save(keys, offsets, oris ,cmps, blockSeg);
}

SSTable::SSTable(const std::vector<Entry> &entries, size_t &pos, const SSTableId &id, TableCache *tableCache)
    : id(id), tableCache(tableCache)
{
    std::vector<uint64_t> keys;
    std::vector<uint64_t> offsets;
    std::vector<uint64_t> oris;
    std::vector<uint64_t> cmps;
    size_t n = entries.size();
    entryCnt = 0;
    blockCnt = 0;
    uint64_t offset = 0;
    uint64_t ori = 0;
    uint64_t cmp = 0;
    std::string block;
    block.reserve(Option::BLOCK_SPACE);
    std::string blockSeg;
    blockSeg.reserve(Option::SST_SPACE);
    uint64_t entryInBlockCnt = 0;
    while (pos < n)
    {
        Entry entry = entries[pos++];
        keys.push_back(entry.key);
        offsets.push_back(offset);
        offset += entry.value.size();
        ++entryCnt;
        block += entry.value;
        ++entryInBlockCnt;
        if (block.size() >= Option::BLOCK_SPACE)
        {
            std::string compressed;
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
    if (entryInBlockCnt > 0)
    {
        std::string compressed;
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
    oris.push_back(ori);
    cmps.push_back(cmp);
    min = keys[0];
    max = keys[entryCnt-1];
    size = cmps.back();
    save(keys, offsets, oris ,cmps, blockSeg);
}

SearchResult SSTable::search(uint64_t key) const
{
    // 현재는 무조건 Disk I/O
    // 이 앞단에 Filter 추가 필요함
    SSTableDataLocation loc = loadAll();

    uint64_t left = 0;
    uint64_t right = entryCnt;
    while (right - left > 2)
    {
        uint64_t mid = left + (right - left) / 2;
        if (loc.keys[mid] < key)
            left = mid;
        else if (loc.keys[mid] > key)
            right = mid;
        else{
            Location location = locate(loc, mid);
            std::string value = loadBlock(loc.cmps,location.pos).substr(location.offset, location.len);

            return {true, locate(loc, mid), value};
        }
            
    }
    if (loc.keys[left] == key){
        Location location = locate(loc, left);
        std::string value = loadBlock(loc.cmps,location.pos).substr(location.offset, location.len);
        return {true, location, value};
    }
    else if (loc.keys[right - 1] == key){
        Location location = locate(loc, right - 1);
        std::string value = loadBlock(loc.cmps,location.pos).substr(location.offset, location.len);
        return {true, location, value};
    }
    else
        return false;
}

SSTableDataLocation SSTable::loadAll() const {
    std::vector<uint64_t> keys;
    std::vector<uint64_t> offsets;
    std::vector<uint64_t> oris;
    std::vector<uint64_t> cmps;

    std::ifstream ifs(id.name(), std::ios::binary);
    ifs.read((char *)&entryCnt, sizeof(uint64_t));
    for (uint64_t i = 0; i <= entryCnt; ++i)
    {
        uint64_t key, offset;
        ifs.read((char *)&key, sizeof(uint64_t));
        ifs.read((char *)&offset, sizeof(uint64_t));
        keys.push_back(key);
        offsets.push_back(offset);
    }
    ifs.read((char *)&blockCnt, sizeof(uint64_t));
    for (uint64_t i = 0; i <= blockCnt; ++i)
    {
        uint64_t ori, cmp;
        ifs.read((char *)&ori, sizeof(uint64_t));
        ifs.read((char *)&cmp, sizeof(uint64_t));
        oris.push_back(ori);
        cmps.push_back(cmp);
    }
    ifs.close();

    return {keys, offsets, oris ,cmps};
}

std::vector<Entry> SSTable::load() const
{
    SSTableDataLocation loc = loadAll();
    std::vector<Entry> entries;
    uint64_t k = 0;
    std::string block = loadBlock(loc.cmps, 0);
    for (uint64_t i = 0; i < entryCnt; ++i)
    {
        if (loc.offsets[i + 1] > loc.oris[k + 1])
            block = loadBlock(loc.cmps, ++k);
        uint64_t key = loc.keys[i];
        std::string value = block.substr(loc.offsets[i] - loc.oris[k], loc.offsets[i + 1] - loc.offsets[i]);
        entries.emplace_back(key, value);
    }
    return entries;
}

void SSTable::remove() const
{
    if (Option::TABLE_CACHE)
        tableCache->close(id);
    std::filesystem::remove(std::filesystem::path(id.name()));
}

uint64_t SSTable::number() const
{
    return id.no;
}

uint64_t SSTable::lower() const
{
    return min;
}

uint64_t SSTable::upper() const
{
    return max;
}

uint64_t SSTable::space() const
{
    return indexSpace() + blockSpace();
}

void SSTable::save(std::vector<uint64_t> keys, std::vector<uint64_t> offsets, std::vector<uint64_t> oris ,std::vector<uint64_t> cmps, const std::string &blockSeg)
{
    std::ofstream ofs(id.name(), std::ios::binary);
    ofs.write((char *)&entryCnt, sizeof(uint64_t));
    for (uint64_t i = 0; i <= entryCnt; ++i)
    {
        ofs.write((char *)&keys[i], sizeof(uint64_t));
        ofs.write((char *)&offsets[i], sizeof(uint64_t));
    }
    ofs.write((char *)&blockCnt, sizeof(uint64_t));
    for (uint64_t i = 0; i <= blockCnt; ++i)
    {
        ofs.write((char *)&oris[i], sizeof(uint64_t));
        ofs.write((char *)&cmps[i], sizeof(uint64_t));
    }
    ofs.write(blockSeg.data(), blockSpace());
    ofs.close();
}

Location SSTable::locate(SSTableDataLocation loc, uint64_t pos) const
{
    uint64_t k = 0;
    while (loc.offsets[pos + 1] > loc.oris[k + 1])
        ++k;
        // sst pos offset len
    return {this, k, loc.offsets[pos] - loc.oris[k], loc.offsets[pos + 1] - loc.offsets[pos]};
}

std::string SSTable::loadBlock(std::vector<uint64_t> cmps, uint64_t pos) const
{
    std::string block;
    char *buf = new char[cmps[pos + 1] - cmps[pos]];
    if (Option::TABLE_CACHE)
    {
        std::ifstream *ifs = tableCache->open(id);
        ifs->seekg(indexSpace() + cmps[pos], std::ios::beg);
        ifs->read(buf, cmps[pos + 1] - cmps[pos]);
    }
    else
    {
        std::ifstream ifs(id.name(), std::ios::binary);
        ifs.seekg(indexSpace() + cmps[pos], std::ios::beg);
        ifs.read(buf, cmps[pos + 1] - cmps[pos]);
        ifs.close();
    }
    block.assign(buf, cmps[pos + 1] - cmps[pos]);
    delete[] buf;
    return block;
}

uint64_t SSTable::indexSpace() const
{
    return (entryCnt * 2 + blockCnt * 2 + 6) * sizeof(uint64_t);
}

uint64_t SSTable::blockSpace() const
{
    return size;
}
