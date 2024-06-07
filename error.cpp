#include "error.hpp"
#include <cerrno>
#include <cstring>
#include <iostream>

std::string Error::createErrorMessage(const std::string& message) {
    saved_errno = errno;
    errno = 0;

    std::string error = "ERROR: " + message;
    if (saved_errno != 0) {
        auto code = std::to_string(saved_errno);
        auto desc = std::strerror(saved_errno);
        error = error + " (" + code + "; " + desc + ")";
    }

    return error;
}

Error::Error(const std::string& message)
    : std::runtime_error(createErrorMessage(message)) {}

#ifdef DEBUG
void debug(const std::string& message) {
    std::cerr << message << std::endl;
}
#else
void debug(const std::string& message) {
    (void)message;
}
#endif
