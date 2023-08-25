#include "../../include/exception/NoEntryFound.h"

NoEntryFoundException::NoEntryFoundException(const char *message) : message(message) {}

const char *NoEntryFoundException::what() const _NOEXCEPT {
    return message.c_str();
}
