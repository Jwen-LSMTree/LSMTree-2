#include "../include/KVStore.h"
#include "../include/Entry.h"

#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <algorithm>
#include <random>
#include <filesystem>

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
    clock_t putTime, seqGetTime, rndGetTime, delTime;
    clock_t s, t;

    if (filesystem::exists(filesystem::path("./data"))) {
        filesystem::remove_all(filesystem::path("./data"));
    }

    KVStore store("./data");

    cout << "Generating data ... ";

    vector<Entry> entries = generate_entries(128 * 1024, 10000, 20000);

    size_t n = entries.size();
    vector<uint64_t> keys;
    for (const Entry &entry: entries) {
        keys.push_back(entry.key);
    }
    cout << "Done." << endl;

    cout << "Processing PUT ... " << endl;
    s = clock();
    for (const Entry &entry: entries) {
        store.put(entry.key, entry.value);
    }
    t = clock();
    putTime = t - s;
    cout << "Done." << endl;

    cout << "Processing GET (seq) ... " << endl;
    s = clock();
    for (uint64_t key: keys) {
        store.get(key);
    }
    t = clock();
    seqGetTime = t - s;
    cout << "Done." << endl;

    vector<uint64_t> rnd = keys;
    shuffle(rnd.begin(), rnd.end(), default_random_engine(time(nullptr)));
    cout << "Processing GET (rnd) ... " << endl;
    s = clock();
    for (uint64_t key: rnd) {
        store.get(key);
    }
    t = clock();
    rndGetTime = t - s;
    cout << "Done." << endl;

//    cout << "Processing DEL ... ";
//    s = clock();
//    for (uint64_t key: keys)
//        store.del(key);
//    t = clock();
//    delTime = t - s;
//    cout << "Done. " << endl;

    cout << fixed << setprecision(4);
    cout << "PUT: " << (double) putTime / n << "ms" << endl;
    cout << "GET (seq): " << (double) seqGetTime / n << "ms" << endl;
    cout << "GET (rnd): " << (double) rndGetTime / n << "ms" << endl;
//    cout << "DEL: " << (double) delTime / n << "ms" << endl;

    return 0;
}
