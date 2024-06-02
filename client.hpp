#ifndef CLIENT_H
#define CLIENT_H

#include "arg_parser.hpp"
#include "network_client.hpp"
#include "protocol_client.hpp"

class Client {
public:
    Client(ClientConfig& config);
    void connectToGame();
    void getCard(const std::string& cardsOnTable);

private:
    ClientNetworker networker;
    ClientProtocol protocol;

    std::string seat;
    bool auto_player;

    std::string hand;
    std::string lastPlayedCard;
};

#endif // CLIENT_H
