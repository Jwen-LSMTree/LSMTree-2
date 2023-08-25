#include "../../include/cache/TableCache.h"

TableCache::~TableCache() {
    for (auto node: linkedList) {
        node.second->close();
        delete node.second;
    }
}

ifstream *TableCache::open(SSTableId id) {
    if (hashMap.count(id.no)) {
        linkedList.push_front(*hashMap[id.no]);
        linkedList.erase(hashMap[id.no]);
        hashMap[id.no] = linkedList.begin();
        return linkedList.front().second;
    } else {
        if (linkedList.size() == Option::TABLE_CACHE_SIZE) {
            hashMap.erase(linkedList.back().first);
            linkedList.back().second->close();
            delete linkedList.back().second;
            linkedList.pop_back();
        }
        ifstream *ifs = new ifstream(id.name(), ios::binary);
        linkedList.emplace_front(id.no, ifs);
        hashMap[id.no] = linkedList.begin();
        return ifs;
    }
}

void TableCache::close(SSTableId id) {
    if (hashMap.count(id.no)) {
        hashMap[id.no]->second->close();
        delete hashMap[id.no]->second;
        linkedList.erase(hashMap[id.no]);
        hashMap.erase(id.no);
    }
}
