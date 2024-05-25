#ifndef NETWORK_CLIENT_H
#define NETWORK_CLIENT_H

#include <string>

class ClientNetworker {
public:
    int sock_fd;
    std::string localAddress;
    std::string peerAddress;
    ClientNetworker(std::string name, std::string service, int domain);
    ~ClientNetworker();
};

#endif // NETWORK_CLIENT_H
