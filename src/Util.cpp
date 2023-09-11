#include "../include/Util.h"

#include <algorithm>

bool compareEntry(Entry &entry1, Entry &entry2);

vector<Entry> Util::compact(const vector<vector<Entry>> &inputs) {
    vector<Entry> entries;
    for (const vector<Entry> &input: inputs) {
        for (const Entry &entry: input) {
            entries.push_back(entry);
        }
    }
    sort(entries.begin(), entries.end(), compareEntry);
    return entries;
}

bool compareEntry(Entry &entry1, Entry &entry2) {
    if (entry1.key == entry2.key) {
        return entry1.seqNum < entry2.seqNum;
    }
    return entry1.key < entry2.key;
}
