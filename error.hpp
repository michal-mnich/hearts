#ifndef ERROR_H
#define ERROR_H

#include <exception>
#include <string>

void debug(const std::string& message);

class SystemError : public std::exception {
private:
    std::string error;

public:
    SystemError(const std::string& message);
    const char* what() const noexcept override;
};

#endif // ERROR_H
