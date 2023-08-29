#include "../../include/disk/SearchResult.h"

#include <utility>

SearchResult::SearchResult(bool success) : success(success) {}

// Object b = new Object();
// Object a = b; // Object b(a)
// move 함수는 위와 같이 객체 전달
SearchResult::SearchResult(bool success, string value)
        : success(success), value(std::move(value)) {}