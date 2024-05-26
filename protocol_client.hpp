#ifndef PROTOCOL_CLIENT_H
#define PROTOCOL_CLIENT_H

#include "network_client.hpp"
#include <fstream>

class ClientProtocol {
private:
    std::ofstream logFile;
    ClientNetworker* networker;

    void logMessage(std::string message, bool incoming);
public:
    ClientProtocol(ClientNetworker* networker);
    ~ClientProtocol();

    void sendIAM(int fd, std::string seat);
    std::string recvBUSY(int fd);
    void recvDEAL();
    void recvTRICK();
    void sendTRICK();
    void recvWRONG();
    void recvTAKEN();
    void recvSCORE();
    void recvTOTAL();
};

#endif // PROTOCOL_CLIENT_H
