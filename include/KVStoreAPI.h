#include <cstdint>
#include <string>

using namespace std;

class KVStoreAPI {
public:
    KVStoreAPI(const string &dir) {}

    KVStoreAPI() = delete;

    virtual void put(uint64_t key, const string &value) = 0;

    virtual string get(uint64_t key) = 0;

//    virtual bool del(uint64_t key) = 0;

    virtual void reset() = 0;
};