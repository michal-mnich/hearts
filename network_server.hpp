#ifndef NETWORK_SERVER_H
#define NETWORK_SERVER_H

#include <arpa/inet.h>
#include <unordered_map>
#include <string>
#include <mutex>

class Server;
class ServerNetworker {
private:
    int ipv4_fd;
    int ipv6_fd;
    struct sockaddr_in ipv4_addr;
    struct sockaddr_in6 ipv6_addr;

    std::mutex mtx;
    std::unordered_map<int, std::pair<std::string, std::string>> clients;

public:
    ServerNetworker(uint16_t port);
    ~ServerNetworker();
    void startAccepting(Server* server);
    void stopAccepting();
    void disconnectAll();
    void removeClient(int fd);
    std::pair<std::string, std::string> getClientInfo(int client_fd);
};

#endif // NETWORK_SERVER_H
