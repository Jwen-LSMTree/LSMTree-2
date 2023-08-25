#pragma once

#include "../include/KVStore.h"

#include <iostream>
#include <cstdint>
#include <string>

using namespace std;

class Test {
protected:
    static const string not_found;

    uint64_t nr_tests;
    uint64_t nr_passed_tests;
    uint64_t nr_phases;
    uint64_t nr_passed_phases;

#define EXPECT(exp, got) expect<decltype(got)>((exp), (got), __FILE__, __LINE__)

    template<typename T>
    void expect(const T &exp, const T &got,
                const string &file, int line) {
        ++nr_tests;
        if (exp == got) {
            ++nr_passed_tests;
            return;
        }
        if (verbose) {
            cerr << "TEST Error @" << file << ":" << line;
            cerr << ", expected " << exp;
            cerr << ", got " << got << endl;
        }
    }

    void phase(void) {
        // Report
        cout << "  Phase " << (nr_phases + 1) << ": ";
        cout << nr_passed_tests << "/" << nr_tests << " ";

        // Count
        ++nr_phases;
        if (nr_tests == nr_passed_tests) {
            ++nr_passed_phases;
            cout << "[PASS]" << endl;
        } else
            cout << "[FAIL]" << endl;

        cout.flush();

        // Reset
        nr_tests = 0;
        nr_passed_tests = 0;
    }

    void report(void) {
        cout << nr_passed_phases << "/" << nr_phases << " passed.";
        cout << endl;
        cout.flush();

        nr_phases = 0;
        nr_passed_phases = 0;
    }

    class KVStore store;

    bool verbose;

public:
    Test(const string &dir, bool v = true) : store(dir), verbose(v) {
        nr_tests = 0;
        nr_passed_tests = 0;
        nr_phases = 0;
        nr_passed_phases = 0;
    }

    virtual void start_test(void *args = NULL) {
        cout << "No test is implemented." << endl;
    }

};

const string Test::not_found = "";
