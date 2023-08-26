#include "../../include/memory/SkipList.h"
#include "../../include/exception/NoEntryFound.h"

#include <iostream>
#include <utility>

SkipList::SkipList() : dist(0, 1) {
    init();
}

SkipList::~SkipList() {
    clear();
    delete head;
    delete tail;
}

void SkipList::clear() {
    for (Tower *tower = head; tower != nullptr;) {
        Tower *next = tower->nexts[0];
        delete tower;
        tower = next;
    }
    init();
}

string SkipList::get(uint64_t key, uint64_t seqNum) const {
    if (!bloomfilter.hasKey(key)) {
        // bloomfilter에 없는 경우 예외 던짐
        throw NoEntryFoundException("no entry found in memory (filtered from BloomFilter)");
    }
    try {
        Tower *tower = get_recentTower(key, seqNum);
        return tower->value;
    } catch (NoEntryFoundException &exception) {
        throw;
    }
}

void SkipList::put(uint64_t key, const string &value, uint64_t seqNum) {
    Tower *prev = get_prevTower(key);

    // 블룸필터에 키 표시
    bloomfilter.insert(key);

    size_t height = 1;
    while (dist(engine)) ++height;
    if (head->height < height + 1) {
        enlargeHeight(height + 1);
    }

    Tower *tower = new Tower(key, value, seqNum, height);
    for (size_t lvl = 0; lvl < height; ++lvl) {
        tower->prevs[lvl] = prev;
        tower->nexts[lvl] = prev->nexts[lvl];
        prev->nexts[lvl]->prevs[lvl] = tower;
        prev->nexts[lvl] = tower;
        while (lvl + 1 >= prev->height)
            prev = prev->prevs[lvl];
    }
    ++totalEntries;
    totalBytes += value.size();
}

// bool SkipList::del(uint64_t key) {
//     Tower *tower = find(key);
//     if (tower == head || tower->key != key)
//         return false;
//     size_t height = tower->height;
//     for (size_t lvl = 0; lvl < height; ++lvl) {
//         tower->prevs[lvl]->nexts[lvl] = tower->nexts[lvl];
//         tower->nexts[lvl]->prevs[lvl] = tower->prevs[lvl];
//     }
//     --totalEntries;
//     totalBytes -= tower->value.size();
//     delete tower;
//     return true;
// }

SkipList::Iterator SkipList::iterator() const {
    return {head->nexts[0]};
}

size_t SkipList::size() const {
    return totalEntries;
}

bool SkipList::empty() const {
    return totalEntries == 0;
}

uint64_t SkipList::space() const {
    return (totalEntries * 3 + totalBytes / Option::BLOCK_SPACE * 2 + 7) * sizeof(uint64_t) + totalBytes;
}

void SkipList::init() {
    head = new Tower(0, "", 0, 1);
    tail = new Tower(UINT64_MAX, "", 0, 1);
    head->prevs[0] = nullptr;
    head->nexts[0] = tail;
    tail->prevs[0] = head;
    tail->nexts[0] = nullptr;
    totalBytes = 0;
    totalEntries = 0;
}

SkipList::Tower *SkipList::get_recentTower(uint64_t key, uint64_t seqNum) const {
    Tower *recent_tower = head;
    Tower *tower = head;
    size_t height = head->height;

    for (size_t i = 1; i <= height; ++i) {
        while (tower->key <= key) {
            if (tower->key == key && tower->seqNum == seqNum) {
                return tower;
            }
            if (tower->key == key && tower->seqNum < seqNum) {
                if (recent_tower->seqNum < tower->seqNum) {
                    recent_tower = tower;
                }
            }
            tower = tower->nexts[height - i];
        }
        while (true) {
            auto prev_tower = tower->prevs[height - i];
            if (prev_tower == nullptr) break;
            tower = prev_tower;
        }
    }
    if (recent_tower == head) {
        throw NoEntryFoundException("no entry found in memory");
    }
    return recent_tower;
}

SkipList::Tower *SkipList::get_prevTower(uint64_t key) const {
    Tower *tower = head;
    size_t height = head->height;
    for (size_t i = 1; i <= height; ++i) {
        while (tower->key <= key) {
            tower = tower->nexts[height - i];
        }
        tower = tower->prevs[height - i];
    }
    return tower;
}

void SkipList::enlargeHeight(size_t height) {
    size_t oldHeight = head->height;
    head->height = height;
    tail->height = height;
    Tower **oldHeadPrevs = head->prevs;
    Tower **oldHeadNexts = head->nexts;
    Tower **oldTailPrevs = tail->prevs;
    Tower **oldTailNexts = tail->nexts;
    head->prevs = new Tower *[height];
    head->nexts = new Tower *[height];
    tail->prevs = new Tower *[height];
    tail->nexts = new Tower *[height];
    for (size_t lvl = 0; lvl < height; ++lvl)
        head->prevs[lvl] = tail->nexts[lvl] = nullptr;
    for (size_t lvl = 0; lvl < oldHeight; ++lvl) {
        head->nexts[lvl] = oldHeadNexts[lvl];
        tail->prevs[lvl] = oldTailPrevs[lvl];
    }
    for (size_t lvl = oldHeight; lvl < height; ++lvl) {
        head->nexts[lvl] = tail;
        tail->prevs[lvl] = head;
    }
    delete[] oldHeadPrevs;
    delete[] oldHeadNexts;
    delete[] oldTailPrevs;
    delete[] oldTailNexts;
}

SkipList::Tower::Tower(uint64_t key, string value, uint64_t seqNum, size_t height)
        : key(key), value(std::move(value)), seqNum(seqNum), height(height) {
    prevs = new Tower *[height];
    nexts = new Tower *[height];
}

SkipList::Tower::~Tower() {
    delete[] prevs;
    delete[] nexts;
}

SkipList::Iterator::Iterator(Tower *tower) : tower(tower) {}

Entry SkipList::Iterator::next() {
    Entry entry(tower->key, tower->value, tower->seqNum);
    if (tower->nexts[0] != nullptr)
        tower = tower->nexts[0];
    return entry;
}

bool SkipList::Iterator::hasNext() const {
    return tower->nexts[0] != nullptr;
}