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
    int fd = networker.sock_fd;

    protocol.sendIAM(fd, seat);

    uint8_t type;
    std::string first;
    protocol.recvDEAL(fd, type, first, hand);
    std::cout << "Deal type: " << (int)type << std::endl;
    std::cout << "First player: " << first << std::endl;
    uint8_t trick, lastPlayedTrick = 0;
    std::string cardsOnTable;
    while (true) {
        try {
            protocol.recvTRICK(fd, &trick, cardsOnTable);
            if (trick == lastPlayedTrick) continue;
        }
        catch (Error& e) {
            std::string message = e.what();
            if (isSubstring(message, "WRONG")) {
                std::cerr << message << std::flush;
                hand.append(lastPlayedCard);
                lastPlayedTrick--;
                continue;
            }
            else {
                throw e;
            }
        }

        std::cout << "Trick: " << (int)trick << std::endl;
        std::cout << "Hand: " << std::endl;
        std::cout << getPrettyCards(hand, true) << std::endl;
        std::cout << "Cards on table (bottom - top): " << std::endl;
        std::cout << getPrettyCards(cardsOnTable) << std::endl;

        getCard(cardsOnTable);
        protocol.sendTRICK(fd, trick, lastPlayedCard);
        lastPlayedTrick = trick;
    }
}

void Client::getCard(const std::string& cardsOnTable) {
    std::string card;
    if (auto_player) {
        if (!cardsOnTable.empty()) {
            card = findCardWithSuit(hand, cardsOnTable.back());
        }
        if (card.empty()) {
            card = getRandomCard(hand);
        }
        sleep(3);
    }
    else {
        std::cout << "Enter your card: ";
        std::cin >> card;
    }
    deleteCard(hand, card);
    lastPlayedCard = card;
}
