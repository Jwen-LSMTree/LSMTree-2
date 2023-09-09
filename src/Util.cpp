#include "../include/Util.h"

#include <algorithm>

bool compareKey(Entry &entry1, Entry &entry2);

vector<Entry> Util::compact(const vector<vector<Entry>> &inputs) {
//    size_t n = inputs.size();
//    vector<Entry> ret;
//    vector<size_t> poses(n);

    vector<Entry> entries;
    for (const vector<Entry> &input: inputs) {
        for (const Entry &entry: input) {
            entries.push_back(entry);
        }
    }
    sort(entries.begin(), entries.end(), compareKey);
    return entries;
}

bool compareKey(Entry &entry1, Entry &entry2) {
    return entry1.key < entry2.key;
}
