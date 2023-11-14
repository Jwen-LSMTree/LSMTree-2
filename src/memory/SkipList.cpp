#include "../../include/memory/SkipList.h"
#include "../../include/exception/NoEntryFound.h"

#include <iostream>
#include <utility>

SkipList::SkipList() : dist(0, 1) {
    init();
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
    bloomFilter = BloomFilter();
    seqNumFilter = SequenceNumberFilter();
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
    if (!bloomFilter.hasKey(key)) {
        throw NoEntryFoundException("no entry found in memory (filtered from BloomFilter)");
    }
    if (!seqNumFilter.isVisible(seqNum)) {
        throw NoEntryFoundException("no entry found in memory (filtered from SequenceNumberFilter");
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
    bloomFilter.insert(key);

    if (seqNum < seqNumFilter.minSeqNum) {
        seqNumFilter.minSeqNum = seqNum;
    }

    size_t height = 1;
    while (dist(random_engine) != 0) { // get random height
        height++;
    }
    if (head->height <= height) {
        enlargeHeadTailHeight(height + 1);
    }

    Node *prevNode = getPreviousNode(key);
    Node *newNode = new Node(key, value, seqNum, height);
    for (size_t level = 0; level < height; level++) {
        newNode->prevs[level] = prevNode;
        newNode->nexts[level] = prevNode->nexts[level];
        prevNode->nexts[level]->prevs[level] = newNode;
        prevNode->nexts[level] = newNode;
        while (level + 1 >= prevNode->height) {
            prevNode = prevNode->prevs[level];
        }
    }
    ++totalEntries;
    totalBytes += value.size();
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

    for (size_t level = 0; level < height; level++) {
        head->prevs[level] = tail->nexts[level] = nullptr;
    }
    for (size_t level = 0; level < oldHeight; level++) {
        head->nexts[level] = oldHeadNexts[level];
        tail->prevs[level] = oldTailPrevs[level];
    }
    for (size_t level = oldHeight; level < height; level++) {
        head->nexts[level] = tail;
        tail->prevs[level] = head;
    }
    delete[] oldHeadPrevs;
    delete[] oldHeadNexts;
    delete[] oldTailPrevs;
    delete[] oldTailNexts;
}

SkipList::Node *SkipList::getNodeBySeqNum(uint64_t key, uint64_t seqNum) const {
    Node *recentNode = head;
    Node *node = head;
    size_t height = head->height;

    for (size_t i = 1; i <= height; i++) {
        while (node->key <= key) {
            if (node->key == key && node->seqNum == seqNum) {
                return node;
            }
            if (node->key == key && node->seqNum < seqNum) {
                if (recentNode->seqNum < node->seqNum) {
                    recentNode = node;
                }
            }
            node = node->nexts[height - i];
        }
        while (true) {
            auto prevNode = node->prevs[height - i];
            if (prevNode == nullptr) {
                break;
            }
            node = prevNode;
        }
    }
    if (recentNode == head) {
        throw NoEntryFoundException("no entry found in memory");
    }
    return recentNode;
}

SkipList::Node *SkipList::getPreviousNode(uint64_t key) const {
    Node *node = head;
    size_t height = head->height;
    for (size_t i = 1; i <= height; i++) {
        while (node->key <= key) {
            node = node->nexts[height - i];
        }
        node = node->prevs[height - i];
    }
    return node;
}

size_t SkipList::size() const {
    return totalEntries;
}

uint64_t SkipList::space() const {
    return totalEntries * 3 * sizeof(uint64_t) + totalBytes;
}

bool SkipList::isEmpty() const {
    return totalEntries == 0;
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

SkipList::Iterator SkipList::iterator() const {
    return {head->nexts[0]};
}

SkipList::Iterator::Iterator(Node *node) : node(node) {}

Entry SkipList::Iterator::next() {
    Entry entry(node->key, node->value, node->seqNum);
    if (node->nexts[0] != nullptr) {
        node = node->nexts[0];
    }
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
