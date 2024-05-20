#ifndef ERROR_H
#define ERROR_H

#include <cerrno>
#include <cstring>
#include <exception>
#include <string>

class SystemError : public std::exception {
  private:
    std::string error;

  public:
    SystemError(const std::string& message) {
        error = "ERROR: " + message;
        if (errno) {
            error = error + " (" + std::to_string(errno) + "; " +
                    std::strerror(errno) + ")";
        }
    }

    const char* what() const noexcept override { return error.c_str(); }
};

#endif // ERROR_H
