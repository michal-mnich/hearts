#ifndef PROTOCOL_CLIENT_H
#define PROTOCOL_CLIENT_H

#include "network_client.hpp"
#include <cstdint>

class ClientProtocol {
private:
    ClientNetworker* networker;
    bool auto_player;

public:
    ClientProtocol(ClientNetworker* networker, bool auto_player);

    void logMessage(std::string message, bool incoming);

    void sendIAM(int fd, std::string seat);
    void recvBUSY(int fd, std::string& taken);
    void
    recvDEAL(int fd, uint8_t& type, std::string& first, std::string& cards);
    void recvTRICK(int fd, uint8_t& trick, std::string& cardsOnTable);
    void sendTRICK(int fd, uint8_t trick, std::string cardPlaced);
    void recvWRONG();
    void recvTAKEN();
    void recvSCORE();
    void recvTOTAL();

    bool tryParseBUSY(const std::string& message, std::string& taken);
    bool tryParseDEAL(const std::string& message,
                      uint8_t& type,
                      std::string& first,
                      std::string& cards);
    bool tryParseTRICK(const std::string& message,
                       uint8_t& trick,
                       std::string& cardsOnTable);
    bool tryParseWRONG(const std::string& message, uint8_t& trick);
    bool tryParseTAKEN(std::string message);
    bool tryParseSCORE(std::string message);
    bool tryParseTOTAL(std::string message);

    bool tryParseInputTRICK(std::string& input, std::string& card);
    bool tryParseInputCards(std::string& input);
    bool tryParseInputTricks(std::string& input);
};

#endif // PROTOCOL_CLIENT_H
