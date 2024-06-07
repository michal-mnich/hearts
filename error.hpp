#ifndef ERROR_H
#define ERROR_H

#include <stdexcept>
#include <string>

class Error : public std::runtime_error {
private:
    std::string createErrorMessage(const std::string& message);

public:
    int saved_errno;
    Error(const std::string& message);
};

void debug(const std::string& message);

#endif // ERROR_H
