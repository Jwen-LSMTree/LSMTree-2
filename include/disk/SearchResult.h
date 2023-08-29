#ifndef SEARCH_RESULT_H
#define SEARCH_RESULT_H

#include "Location.h"

#include <string>

using namespace std;

struct SearchResult {
    bool success;
    string value;

    SearchResult(bool success);

    SearchResult(bool success, std::string value);
};

#endif