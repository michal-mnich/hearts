#ifndef CLIENT_H
#define CLIENT_H

#include "network_client.hpp"
#include "protocol.hpp"
#include <string>

class Client {
public:
    Client(std::string host, std::string port, int domain);
    void connectToGame();

private:
    ClientNetworker networker;
    ClientProtocol protocol;
};

#endif // CLIENT_H
