#ifndef SKIP_LIST_H
#define SKIP_LIST_H

#include "../bloom_filter/BloomFilter.h"
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

    explicit SkipList();

    ~SkipList();

    string get(uint64_t key, uint64_t seqNum) const;

    void put(uint64_t key, const string &value, uint64_t seqNum);

//    bool del(uint64_t key);

    Iterator iterator() const;

    size_t size() const;

    bool empty() const;

    void clear();

    uint64_t space() const;

    void print() const;

    BloomFilter bloomfilter;

private:
    struct Node;
    Node *head, *tail;
    size_t totalEntries;
    size_t totalBytes;
    std::default_random_engine engine;
    std::uniform_int_distribution<int> dist;

    void init();

    Node *getNodeBySeqNum(uint64_t key, uint64_t seqNum) const;

    Node *getNode(uint64_t key) const;

    void enlargeHeight(size_t height);
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