#ifndef ERROR_H
#define ERROR_H

#include <exception>
#include <string>

class Error : public std::exception {
private:
    std::string error;

public:
    Error(const std::string& message);
    const char* what() const noexcept override;
};

void debug(const std::string& message);

#endif // ERROR_H
