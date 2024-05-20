#ifndef NETWORK_H
#define NETWORK_H

#include <arpa/inet.h>
#include <cstring>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
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
  protected:
    virtual void createSocket() = 0;
    virtual void bindSocket() = 0;
    int _createSocket(int domain);
    void _bindSocket(int sock_fd, struct sockaddr* addr, socklen_t addr_size);

  public:
    ServerNetworker(uint16_t port);
    void listenSocket();
};

class IPv4ServerNetworker : public ServerNetworker {
  private:
    struct sockaddr_in addr;

  public:
    IPv4ServerNetworker(uint16_t port);
    void createSocket() override;
    void bindSocket() override;
};

class IPv6ServerNetworker : public ServerNetworker {
  private:
    struct sockaddr_in6 addr;

  public:
    IPv6ServerNetworker(uint16_t port);
    void createSocket() override;
    void bindSocket() override;
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
