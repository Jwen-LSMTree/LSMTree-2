#include "SearchResult.h"

#include <utility>

SearchResult::SearchResult(bool success) : success(success) {}

SearchResult::SearchResult(bool success, Location location)
        : success(success), location(location) {}

SearchResult::SearchResult(bool success, Location location, string value)
        : success(success), location(location), value(std::move(value)) {}