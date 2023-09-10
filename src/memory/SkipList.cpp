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
    for (Node *node = head; node != nullptr;) {
        Node *next = node->nexts[0];
        delete node;
        node = next;
    }
    init();
}

string SkipList::get(uint64_t key, uint64_t seqNum) const {
    if (!bloomfilter.hasKey(key)) {
        // bloomfilter에 없는 경우 예외 던짐
        throw NoEntryFoundException("no entry found in memory (filtered from BloomFilter)");
    }
    try {
        Node *node = getNodeBySeqNum(key, seqNum);
        return node->value;
    }
    catch (NoEntryFoundException &exception) {
        throw;
    }
}

void SkipList::put(uint64_t key, const string &value, uint64_t seqNum) {
    Node *prev = getPreviousNode(key);

    bloomfilter.insert(key);

    size_t height = 1;
    while (dist(engine))
        ++height;
    if (head->height < height + 1) {
        enlargeHeadTailHeight(height + 1);
    }

    Node *newNode = new Node(key, value, seqNum, height);
    for (size_t level = 0; level < height; ++level) {
        newNode->prevs[level] = prev;
        newNode->nexts[level] = prev->nexts[level];
        prev->nexts[level]->prevs[level] = newNode;
        prev->nexts[level] = newNode;
        while (level + 1 >= prev->height)
            prev = prev->prevs[level];
    }
    ++totalEntries;
    totalBytes += value.size();
}

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
    return (totalEntries * 3 + totalBytes / Option::BLOCK_SPACE * 1 + 6) * sizeof(uint64_t) + totalBytes;
}

void SkipList::init() {
    head = new Node(0, "", 0, 1);
    tail = new Node(UINT64_MAX, "", 0, 1);
    head->prevs[0] = nullptr;
    head->nexts[0] = tail;
    tail->prevs[0] = head;
    tail->nexts[0] = nullptr;
    totalBytes = 0;
    totalEntries = 0;
    bloomfilter = BloomFilter();
}

SkipList::Node *SkipList::getNodeBySeqNum(uint64_t key, uint64_t seqNum) const {
    Node *recent_node = head;
    Node *node = head;
    size_t height = head->height;

    for (size_t i = 1; i <= height; ++i) {
        while (node->key <= key) {
            if (node->key == key && node->seqNum == seqNum) {
                return node;
            }
            if (node->key == key && node->seqNum < seqNum) {
                if (recent_node->seqNum < node->seqNum) {
                    recent_node = node;
                }
            }
            node = node->nexts[height - i];
        }
        while (true) {
            auto prev_node = node->prevs[height - i];
            if (prev_node == nullptr)
                break;
            node = prev_node;
        }
    }
    if (recent_node == head) {
        throw NoEntryFoundException("no entry found in memory");
    }
    return recent_node;
}

SkipList::Node *SkipList::getPreviousNode(uint64_t key) const {
    Node *node = head;
    size_t height = head->height;
    for (size_t i = 1; i <= height; ++i) {
        while (node->key <= key) {
            node = node->nexts[height - i];
        }
        node = node->prevs[height - i];
    }
    return node;
}

void SkipList::enlargeHeadTailHeight(size_t height) {
    size_t oldHeight = head->height;
    head->height = height;
    tail->height = height;
    Node **oldHeadPrevs = head->prevs;
    Node **oldHeadNexts = head->nexts;
    Node **oldTailPrevs = tail->prevs;
    Node **oldTailNexts = tail->nexts;
    head->prevs = new Node *[height];
    head->nexts = new Node *[height];
    tail->prevs = new Node *[height];
    tail->nexts = new Node *[height];
    for (size_t level = 0; level < height; ++level)
        head->prevs[level] = tail->nexts[level] = nullptr;
    for (size_t level = 0; level < oldHeight; ++level) {
        head->nexts[level] = oldHeadNexts[level];
        tail->prevs[level] = oldTailPrevs[level];
    }
    for (size_t level = oldHeight; level < height; ++level) {
        head->nexts[level] = tail;
        tail->prevs[level] = head;
    }
    delete[] oldHeadPrevs;
    delete[] oldHeadNexts;
    delete[] oldTailPrevs;
    delete[] oldTailNexts;
}

SkipList::Node::Node(uint64_t key, string value, uint64_t seqNum, size_t height)
        : key(key), value(std::move(value)), seqNum(seqNum), height(height) {
    prevs = new Node *[height];
    nexts = new Node *[height];
}

SkipList::Node::~Node() {
    delete[] prevs;
    delete[] nexts;
}

SkipList::Iterator::Iterator(Node *node) : node(node) {}

Entry SkipList::Iterator::next() {
    Entry entry(node->key, node->value, node->seqNum);
    if (node->nexts[0] != nullptr)
        node = node->nexts[0];
    return entry;
}

bool SkipList::Iterator::hasNext() const {
    return node->nexts[0] != nullptr;
}

void SkipList::print() const {
    cout << "=== SkipList === " << endl;
    Iterator itr = iterator();
    while (itr.hasNext()) {
        Entry entry = itr.next();
        cout << "key: " << entry.key << ", value: " << entry.value << ", seqNum: " << entry.seqNum << endl;
    }
    cout << "\n" << endl;
}
