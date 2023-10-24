#ifndef NO_ENTRY_FOUND
#define NO_ENTRY_FOUND

#include <string>
#include <exception>

using namespace std;

class NoEntryFoundException : public exception {
public:
    string message;

    explicit NoEntryFoundException(const char *message);

    const char *what() const noexcept override;
};


#endif
