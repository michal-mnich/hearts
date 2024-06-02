#ifndef PROTOCOL_SERVER_H
#define PROTOCOL_SERVER_H

#include "network_server.hpp"
#include <string>
#include <map>

class ServerProtocol {
private:
    ServerNetworker* networker;

    void logMessage(int client_fd, std::string message, bool incoming);

public:
    unsigned int timeout;
    ServerProtocol(ServerNetworker* networker, unsigned int timeout);

    void recvIAM(int fd, std::string& seat);
    void sendBUSY(int fd, std::string taken);
    void sendDEAL(int fd, uint8_t type, std::string first, std::string cards);
    void sendTRICK(int fd, uint8_t trick, std::string cardsOnTable);
    void recvTRICK(int fd, uint8_t& trick, std::string& cardPlaced);
    void sendWRONG(int fd, uint8_t trick);
    void sendTAKEN(int fd,
                   uint8_t trick,
                   std::string& cardsTaken,
                   std::string& seat);
    void sendSCORE(int fd, std::map<std::string, unsigned int>& scores);
    void sendTOTAL(int fd, std::map<std::string, unsigned int>& totals);

    bool tryParseIAM(const std::string& message, std::string& seat);
    bool tryParseTRICK(const std::string& message,
                       uint8_t& trick,
                       std::string& cardPlaced);
};

#endif // PROTOCOL_SERVER_H
