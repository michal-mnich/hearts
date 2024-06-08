#ifndef PROTOCOL_CLIENT_H
#define PROTOCOL_CLIENT_H

#include "network_client.hpp"
#include <cstdint>
#include <map>

class ClientProtocol {
private:
    ClientNetworker* networker;

public:
    bool auto_player;
    ClientProtocol(ClientNetworker* networker, bool auto_player);

    void logMessage(const std::string& message, bool incoming);

    void sendIAM(int fd, const std::string& seat);
    void sendTRICK(int fd, uint8_t trick, const std::string& cardPlaced);

    bool tryParseBUSY(const std::string& message, std::string& taken);
    bool tryParseDEAL(const std::string& message,
                      uint8_t& type,
                      std::string& first,
                      std::string& cards);
    bool tryParseTRICK(const std::string& message,
                       uint8_t& trick,
                       std::string& cardsOnTable);
    bool tryParseWRONG(const std::string& message, uint8_t& trick);

    bool tryParseTAKEN(const std::string& message,
                       uint8_t& trick,
                       std::string& cardsTaken,
                       std::string& seat);
    bool tryParseSCORE(const std::string& message,
                       std::map<std::string, unsigned int>& scores);
    bool tryParseTOTAL(const std::string& message,
                       std::map<std::string, unsigned int>& totals);

    bool tryParseInputTRICK(const std::string& input, std::string& card);
    bool tryParseInputCards(const std::string& input);
    bool tryParseInputTricks(const std::string& input);

    void displayBUSY(const std::string& taken);
    void displayDEAL(uint8_t type,
                     const std::string& first,
                     const std::string& hand);
    void displayWRONG(uint8_t trick);
    void displayTAKEN(uint8_t trick,
                      const std::string& cardsTaken,
                      const std::string& takingPlayer);
    void displaySCORE(const std::map<std::string, unsigned int>& scores);
    void displayTOTAL(const std::map<std::string, unsigned int>& totals);
    void displayTRICK(uint8_t trick,
                      const std::string& cardsOnTable,
                      const std::string& hand);
};

#endif // PROTOCOL_CLIENT_H
