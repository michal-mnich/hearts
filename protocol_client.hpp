#ifndef PROTOCOL_CLIENT_H
#define PROTOCOL_CLIENT_H

#include "network_client.hpp"
#include <inttypes.h>

class ClientProtocol {
private:
ClientNetworker* networker;

    void logMessage(std::string message, bool incoming);
public:
    ClientProtocol(ClientNetworker* networker);

    void sendIAM(int fd, std::string seat);
    void recvBUSY(int fd, std::string& taken);
    void recvDEAL(int fd, uint8_t& type, std::string& first, std::string& cards);
    void recvTRICK(int fd, uint8_t& trick, std::string& cardsOnTable);
    void sendTRICK(int fd, uint8_t trick, std::string cardPlaced);
    void recvWRONG();
    void recvTAKEN();
    void recvSCORE();
    void recvTOTAL();
};

#endif // PROTOCOL_CLIENT_H
