#include "Entry.h"
#include "KVStore.h"

#include <iostream>
#include <vector>
#include <filesystem>
#include <ctime>

using namespace std;

vector<Entry> generate_entries(size_t n, size_t lo, size_t hi) {
    vector<Entry> entries;
    default_random_engine eng(time(nullptr));
    uniform_int_distribution<size_t> distSizeT(lo, hi);
    uniform_int_distribution<int> distChar('a', 'z');

    for (uint64_t key = 0; key < n; ++key) {
        string value;
        size_t len = distSizeT(eng);
        for (size_t i = 0; i < len; ++i)
            value += distChar(eng);
        entries.emplace_back(key, value);
    }
    return entries;
}

int main() {
    if (filesystem::exists(filesystem::path("./data")))
        filesystem::remove_all(filesystem::path("./data"));

    clock_t s, t;
    KVStore store("./data");

    size_t n = 120 * 1024;
    cout << "Generating data ... ";
    vector<Entry> entries = generate_entries(n, 8192, 8192);
    cout << "Done." << endl;

    vector<double> result(n / 1024);

    int roundCnt = 25;
    for (int k = 0; k < roundCnt; ++k) {
        cout << "Processing round " << k << " ... ";
        store.reset();
        for (size_t i = 0; i < n; i += 1024) {
            s = clock();
            for (size_t k = 0; k < 1024; ++k)
                store.put(entries[i + k].key, entries[i + k].value);
            t = clock();
            result[i / 1024] += (double) (t - s) / roundCnt;
        }
        cout << "Done." << endl;
    }

    for (size_t i = 0; i < n; i += 1024)
        cout << result[i / 1024] << " ";
    cout << endl;

    return 0;
}
