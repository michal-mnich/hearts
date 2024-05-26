#ifndef PROTOCOL_SERVER_H
#define PROTOCOL_SERVER_H

#include "network_server.hpp"
#include <string>

class ServerProtocol {
private:
    ServerNetworker* networker;
    unsigned int timeout;

    void logMessage(int client_fd, std::string message, bool incoming);
public:
    ServerProtocol(ServerNetworker* networker, unsigned int timeout);

    std::string recvIAM(int fd);
    void sendBUSY(int fd, std::string taken);
    void sendDEAL();
    void sendTRICK();
    void recvTRICK();
    void sendWRONG();
    void sendTAKEN();
    void sendSCORE();
    void sendTOTAL();
};

#endif // PROTOCOL_SERVER_H
