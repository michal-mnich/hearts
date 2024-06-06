#ifndef CLIENT_H
#define CLIENT_H

#include "arg_parser.hpp"
#include "network_client.hpp"
#include "protocol_client.hpp"

class Client {
public:
    explicit Client(ClientConfig& config);
    bool connectToGame();

private:
    ClientNetworker networker;
    ClientProtocol protocol;

    std::string seat;

    bool lastTotal = false;
    std::string cardsOnTable;
    std::string hand;
    std::string lastPlayedCard;
    uint8_t lastPlayedTrick = 0;
    uint8_t serverTrick = 0;

    std::vector<std::string> tricksTaken;

    std::string getAutoCard();

    void handleServer();
    void handleInput();
};

#endif // CLIENT_H
