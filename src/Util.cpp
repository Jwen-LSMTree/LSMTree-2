#include "../include/Util.h"

#include <algorithm>

bool compareKey(Entry &e1, Entry &e2);

vector<Entry> Util::compact(const vector<vector<Entry>> &inputs) {
    size_t n = inputs.size();
    vector<Entry> ret;
    vector<size_t> poses(n);

    vector<Entry> entries;
    for (const vector<Entry> &input: inputs) {
        for (const Entry &entry: input) {
            entries.push_back(entry);
        }
    }
    sort(entries.begin(), entries.end(), compareKey);
    return entries;
}

bool compareKey(Entry &e1, Entry &e2) {
    return e1.key < e2.key;
}
