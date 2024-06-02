#ifndef CLIENT_H
#define CLIENT_H

#include "arg_parser.hpp"
#include "network_client.hpp"
#include "protocol_client.hpp"

class Client {
public:
    Client(ClientConfig& config);
    void connectToGame();
    std::string getAutoCard(const std::string& cardsOnTable);

private:
    ClientNetworker networker;
    ClientProtocol protocol;

    std::string seat;
    bool auto_player;

    std::string hand;
    std::string lastPlayedCard;

    std::vector<std::string> tricksTaken;
};

#endif // CLIENT_H
