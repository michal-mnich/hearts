#ifndef ERROR_H
#define ERROR_H

#include <cerrno>
#include <cstring>
#include <exception>
#include <string>

class NetworkError : public std::exception {
  private:
    std::string error;

  public:
    NetworkError(const std::string& message)
        : error("ERROR: " + message + ": " + std::strerror(errno)) {}

    const char* what() const noexcept override { return error.c_str(); }
};

#endif // ERROR_H
