#ifndef CLIENT_H
#define CLIENT_H

#include "network.hpp"
#include <string>

class Client {
  public:
    Client(const std::string& host, int port, int domain);
    void connectToGame();

  private:
    ClientNetworker networker;
};

#endif // CLIENT_H
