#include "client.hpp"
#include "common.hpp"
#include "error.hpp"
#include "protocol_client.hpp"
#include <algorithm>
#include <iostream>

Client::Client(ClientConfig& config)
    : networker(config.host, config.port, config.domain), protocol(&networker),
      seat(config.seat), auto_player(config.auto_player) {}

void Client::connectToGame() {
    try {
        int fd = networker.sock_fd;

        protocol.sendIAM(fd, seat);

        uint8_t type;
        std::string first;
        protocol.recvDEAL(fd, type, first, hand);
        uint8_t trick;
        std::string cardsOnTable;
        while (true) {
            protocol.recvTRICK(fd, &trick, cardsOnTable);
            std::cout << "Trick " << (int)trick << ": " << cardsOnTable
                      << std::endl;
            protocol.sendTRICK(fd, trick, getCard(cardsOnTable));
        }
    }
    catch (Error& e) {
        std::cerr << e.what() << std::endl;
    }
}

std::string Client::getCard(std::string cardsOnTable) {
    std::string card;
    if (auto_player) {
        if (cardsOnTable.empty()) {
            card = hand.substr(0, 2);
            hand.erase(0, 2);
        }
        else {
            char lastColor = cardsOnTable.back();
            auto it = std::find(hand.begin(), hand.end(), lastColor);
            if (it == hand.end()) {
                card = hand.substr(0, 2);
                hand.erase(0, 2);
            }
            else {
                it--;
                if (*it != '0') {
                    card = hand.substr(it - hand.begin(), 2);
                    hand.erase(it - hand.begin(), 2);
                }
                else {
                    it--;
                    card = hand.substr(it - hand.begin(), 3);
                    hand.erase(it - hand.begin(), 3);
                }
            }
        }
        sleep(5);
    }
    else {
        std::cout << "Enter your card: ";
        std::cin >> card;
    }
    return card;
}
