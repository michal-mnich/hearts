#ifndef NETWORK_H
#define NETWORK_H

#include <arpa/inet.h>
#include <condition_variable>
#include <mutex>
#include <netdb.h>
#include <string>
#include <thread>
#include <unistd.h>
#include <unordered_set>

#define MAX_CLIENTS 4

std::string getPeerAddress(int sock_fd);
std::string getLocalAddress(int sock_fd);

class Server;

class ServerNetworker {
private:
    int ipv4_fd;
    int ipv6_fd;
    struct sockaddr_in ipv4_addr;
    struct sockaddr_in6 ipv6_addr;

    std::unordered_set<int> client_fds;

    std::mutex mtx;

    int active_clients;
    std::condition_variable active_clients_cv;

public:
    ServerNetworker(uint16_t port);
    ~ServerNetworker();
    void startAccepting(Server* server);
    void stopAccepting();
    void disconnectClients();
};

class ClientNetworker {
public:
    int sock_fd;
    ClientNetworker(std::string name, std::string service, int domain);
    ~ClientNetworker();
};

#endif // NETWORK_H
