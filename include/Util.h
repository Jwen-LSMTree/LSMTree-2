#ifndef UTIL_H
#define UTIL_H

#include "Entry.h"

#include <vector>

using namespace std;

namespace Util {
    vector<Entry> compact(const vector<vector<Entry>> &inputs);
}

#endif
