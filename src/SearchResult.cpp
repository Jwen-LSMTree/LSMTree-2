#include "SearchResult.h"

SearchResult::SearchResult(bool success): success(success) {}

SearchResult::SearchResult(bool success, Location location)
    : success(success), location(location) {}

SearchResult::SearchResult(bool success, Location location, std::string value)
    : success(success), location(location), value(value) {}