#include "Test.h"

#include <iostream>
#include <string>
#include <ctime>
#include <filesystem>

using namespace std;

class CorrectnessTest : public Test {
private:
    const uint64_t SIMPLE_TEST_MAX = 512;
    const uint64_t LARGE_TEST_MAX = 1024 * 64;

    void regular_test(uint64_t max) {
        uint64_t i;

        store.put(1, "SE");
        EXPECT("SE", store.get(1));

        phase();

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
//        for (i = 0; i < max; i += 2)
//            EXPECT(true, store.del(i));



//        for (i = 1; i < max; ++i)
//            EXPECT(i & 1, store.del(i));

        phase();

        report();
    }

public:
    CorrectnessTest(const string &dir, bool v = true) : Test(dir, v) {
    }

    void start_test(void *args = NULL) override {
        cout << "KVStore Correctness Test" << endl;

        cout << "[Simple Test]" << endl;
        regular_test(SIMPLE_TEST_MAX);

        cout << "[Large Test]" << endl;
        regular_test(LARGE_TEST_MAX);
    }
};

int main(int argc, char *argv[]) {
    if (filesystem::exists(filesystem::path("./data"))) {
        filesystem::remove_all(filesystem::path("./data"));
    }

    bool verbose = (argc == 2 && string(argv[1]) == "-v");

    cout << "Usage: " << argv[0] << " [-v]" << endl;
    cout << "  -v: print extra info for failed tests [currently ";
    cout << (verbose ? "ON" : "OFF") << "]" << endl;
    cout << endl;
    cout.flush();

    CorrectnessTest test("./data", verbose);

    clock_t a = clock();
    test.start_test();
    clock_t b = clock();
    cout << "Total time: " << (b - a) << "ms" << endl;

    return 0;
}
