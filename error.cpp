#include "error.hpp"
#include <cerrno>
#include <cstring>
#include <iostream>

Error::Error(const std::string& message) {
    error = "ERROR: " + message;
    if (errno) {
        auto code = std::to_string(errno);
        auto desc = std::strerror(errno);
        error = error + " (" + code + "; " + desc + ")";
        errno = 0;
    }
}

const char* Error::what() const noexcept {
    return error.c_str();
}

#ifdef DEBUG
void debug(const std::string& message) {
    std::cerr << message << std::endl;
}
#else
void debug(const std::string& message) {
    (void)message;
}
#endif
