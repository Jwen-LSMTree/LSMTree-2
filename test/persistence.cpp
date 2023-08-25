#include "Test.h"

#include <iostream>
#include <string>
#include <cassert>

using namespace std;

class PersistenceTest : public Test {
private:
    const uint64_t TEST_MAX = 1024 * 32;

    void prepare(uint64_t max) {
        uint64_t i;

        // Clean up
        store.reset();

        // Test multiple key-value pairs
        for (i = 0; i < max; ++i) {
            store.put(i, string(i + 1, 's'));
            EXPECT(string(i + 1, 's'), store.get(i));
        }
        phase();

        // Test after all insertions
        for (i = 0; i < max; ++i)
            EXPECT(string(i + 1, 's'), store.get(i));
        phase();

        // Test deletions
//		for (i = 0; i < max; i+=2)
//			EXPECT(true, store.del(i));

        // Prepare data for Test Mode
        for (i = 0; i < max; ++i) {
            switch (i & 3) {
                case 0:
                    EXPECT(not_found, store.get(i));
                    store.put(i, string(i + 1, 't'));
                    break;
                case 1:
                    EXPECT(string(i + 1, 's'), store.get(i));
                    store.put(i, string(i + 1, 't'));
                    break;
                case 2:
                    EXPECT(not_found, store.get(i));
                    break;
                case 3:
                    EXPECT(string(i + 1, 's'), store.get(i));
                    break;
                default:
                    assert(0);
            }
        }

        phase();

        report();

        /**
         * Write 10MB data to drain previous data out of memory.
         */
        for (i = 0; i <= 10240; ++i)
            store.put(max + i, string(1024, 'x'));

        cout << "Data is ready, please press ctrl-c/ctrl-d to"
                " terminate this program!" << endl;
        cout.flush();

        while (true) {
            volatile int dummy;
            for (i = 0; i <= 1024; ++i) {
                // The loop slows down the program
                for (i = 0; i <= 1000; ++i)
                    dummy = i;

//                store.del(max + i);

                for (i = 0; i <= 1000; ++i)
                    dummy = i;

                store.put(max + i, string(1024, '.'));

                for (i = 0; i <= 1000; ++i)
                    dummy = i;

                store.put(max + i, string(512, 'x'));
            }
        }
    }

    void test(uint64_t max) {
        uint64_t i;
        // Test data
        for (i = 0; i < max; ++i) {
            switch (i & 3) {
                case 0:
                    EXPECT(string(i + 1, 't'), store.get(i));
                    break;
                case 1:
                    EXPECT(string(i + 1, 't'), store.get(i));
                    break;
                case 2:
                    EXPECT(not_found, store.get(i));
                    break;
                case 3:
                    EXPECT(string(i + 1, 's'), store.get(i));
                    break;
                default:
                    assert(0);
            }
        }

        phase();

        report();
    }

public:
    PersistenceTest(const string &dir, bool v = true) : Test(dir, v) {
    }

    void start_test(void *args = NULL) override {
        bool testmode = (args && *static_cast<bool *>(args));

        cout << "KVStore Persistence Test" << endl;

        if (testmode) {
            cout << "<<Test Mode>>" << endl;
            test(TEST_MAX);
        } else {
            cout << "<<Preparation Mode>>" << endl;
            prepare(TEST_MAX);
        }
    }
};

void usage(const char *prog, const char *verb, const char *mode) {
    cout << "Usage: " << prog << " [-t] [-v]" << endl;
    cout << "  -t: test mode for persistence test,"
            " if -t is not given, the program only prepares data for test."
            " [currently " << mode << "]" << endl;
    cout << "  -v: print extra info for failed tests [currently ";
    cout << verb << "]" << endl;
    cout << endl;
    cout << " NOTE: A normal usage is as follows:" << endl;
    cout << "    1. invoke `" << prog << "`;" << endl;
    cout << "    2. terminate (kill) the program when data is ready;";
    cout << endl;
    cout << "    3. invoke `" << prog << "-t ` to test." << endl;
    cout << endl;
    cout.flush();
}


int main(int argc, char *argv[]) {
    bool verbose = false;
    bool testmode = false;

    if (argc == 2) {
        verbose = string(argv[1]) == "-v";
        testmode = string(argv[1]) == "-t";
    } else if (argc == 3) {
        verbose = string(argv[1]) == "-v" ||
                  string(argv[2]) == "-v";
        testmode = string(argv[1]) == "-t" ||
                   string(argv[2]) == "-t";
    } else if (argc > 3) {
        cerr << "Too many arguments." << endl;
        usage(argv[0], "OFF", "Preparation Mode");
        exit(-1);
    }
    usage(argv[0], verbose ? "ON" : "OFF",
          testmode ? "Test Mode" : "Preparation Mode");

    PersistenceTest test("./data", verbose);

    test.start_test(static_cast<void *>(&testmode));

    return 0;
}
