#ifndef NETWORK_H
#define NETWORK_H

#include <arpa/inet.h>
#include <netdb.h>
#include <string>
#include <unistd.h>

std::string getPeerAddress(int sock_fd);
std::string getLocalAddress(int sock_fd);

void closeSocket(int sock_fd);
void shutdownSocket(int sock_fd);

class Networker {
protected:
    int sock_fd;
    uint16_t port;

public:
    Networker(uint16_t port);
    int getSocket();
};

class ServerNetworker : public Networker {
private:
    struct sockaddr_in6 addr;

public:
    ServerNetworker(uint16_t port);
    void createSocket();
    void bindSocket();
    void listenSocket();
};

class ClientNetworker : public Networker {
private:
    std::string host;
    int domain;

public:
    ClientNetworker(std::string host, uint16_t port, int domain);
    void connectToServer();
};

#endif // NETWORK_H
