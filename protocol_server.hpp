#ifndef PROTOCOL_SERVER_H
#define PROTOCOL_SERVER_H

#include "network_server.hpp"
#include <string>
#include <fstream>

class ServerProtocol {
private:
    std::ofstream logFile;
    ServerNetworker* networker;
    unsigned int timeout;

    void logMessage(int client_fd, std::string message, bool incoming);
public:
    ServerProtocol(ServerNetworker* networker, unsigned int timeout);
    ~ServerProtocol();

    std::string recvIAM(int fd);
    void sendBUSY(int fd);
    void sendDEAL();
    void sendTRICK();
    void recvTRICK();
    void sendWRONG();
    void sendTAKEN();
    void sendSCORE();
    void sendTOTAL();
};

#endif // PROTOCOL_SERVER_H
