#ifndef SKIP_LIST_H
#define SKIP_LIST_H

#include "../bloom_filter/BloomFilter.h"
#include "../sequence_number_filter/SequenceNumberFilter.h"
#include "../Entry.h"
#include "../Option.h"

#include <cstdint>
#include <cstddef>
#include <string>
#include <random>
#include <utility>

using namespace std;

class SkipList {
public:
    class Iterator;

    BloomFilter bloomFilter;

    explicit SkipList();

    ~SkipList();

    void clear();

    string get(uint64_t key, uint64_t seqNum) const;

    void put(uint64_t key, const string &value, uint64_t seqNum);

    size_t size() const;

    uint64_t space() const;

    bool isEmpty() const;

    void print() const;

    Iterator iterator() const;

    SequenceNumberFilter seqNumFilter;

private:
    struct Node;
    Node *head, *tail;

    size_t totalEntries;
    size_t totalBytes;

    default_random_engine random_engine; // 난수 생성기
    uniform_int_distribution<int> dist;

    void init();

    void enlargeHeadTailHeight(size_t height);

    Node *getNodeBySeqNum(uint64_t key, uint64_t seqNum) const;

    Node *getPreviousNode(uint64_t key) const;
};

struct SkipList::Node {
    uint64_t key;
    string value;
    uint64_t seqNum;

    Node **prevs;
    Node **nexts;
    size_t height;

    explicit Node(uint64_t key, string value, uint64_t seqNum, size_t height);

    Node() = delete;

    ~Node();
};

class SkipList::Iterator {
public:
    Entry next();

    bool hasNext() const;

private:
    Node *node;

    Iterator(Node *node);

    friend SkipList;
};

#endif