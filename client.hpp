#ifndef CLIENT_H
#define CLIENT_H

#include "network.hpp"
#include <string>

class Client {
public:
    Client(std::string host, std::string port, int domain);
    void connectToGame();

private:
    ClientNetworker networker;
};

#endif // CLIENT_H
