#include "error.hpp"
#include <cerrno>
#include <cstring>
#include <iostream>

std::string createErrorMessage(const std::string& message) {
    std::string error = "ERROR: " + message;
    if (errno) {
        auto code = std::to_string(errno);
        auto desc = std::strerror(errno);
        error = error + " (" + code + "; " + desc + ")";
        errno = 0;
    }
    return error;
}

Error::Error(const std::string& message)
    : saved_errno(errno), std::runtime_error(createErrorMessage(message)) {}

#ifdef DEBUG
void debug(const std::string& message) {
    std::cerr << message << std::endl;
}
#else
void debug(const std::string& message) {
    (void)message;
}
#endif
