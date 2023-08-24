#ifndef SEARCH_RESULT_H
#define SEARCH_RESULT_H

#include "Location.h"

#include <string>

using namespace std;

struct SearchResult {
    bool success;
    Location location;
    string value;

    SearchResult(bool success);

    SearchResult(bool success, Location location);
};

#endif