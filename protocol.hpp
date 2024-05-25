#include "rules.hpp"
#include <string>

class IAMMessage {
private:
    Seat seat;

public:
    IAMMessage(Seat s) : seat(s) {}
    std::string toString();
    static bool isValid(std::string msg);
};

class BUSYMessage {

public:
    std::string toString();
    static bool isValid(std::string msg);
};
