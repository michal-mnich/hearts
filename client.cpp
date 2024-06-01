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
    uint8_t trick;
    std::string cardsOnTable;
    while (true) {
        try {
            protocol.recvTRICK(fd, &trick, cardsOnTable);
        }
        catch (Error& e) {
            std::string message = e.what();
            std::cerr << message << std::flush;
            if (isSubstring(message, "invalid TRICK message")) {
                hand.append(playedCard);
                continue;
            }
            else {
                break;
            }
        }

        std::cout << "Trick: " << (int)trick << std::endl;
        std::cout << "Hand: " << std::endl;
        std::cout << getPrettyCards(hand) << std::endl;
        std::cout << "Cards on table (bottom-top): " << std::endl;
        std::cout << getPrettyCards(cardsOnTable) << std::endl;

        getCard(cardsOnTable);
        protocol.sendTRICK(fd, trick, playedCard);
    }
}

std::string getRandomCard(std::string& hand) {
    std::string card;
    srand(time(NULL));
    int index = rand() % hand.size();
    if (hand[index] == 'S' || hand[index] == 'H' || hand[index] == 'D' ||
        hand[index] == 'C')
    {
        index--;
    }
    if (hand[index] == '0') index--;
    if (hand[index] == '1') {
        card = hand.substr(index, 3);
        hand.erase(index, 3);
    }
    else {
        card = hand.substr(index, 2);
        hand.erase(index, 2);
    }
    return card;
}

void Client::getCard(std::string cardsOnTable) {
    std::string card;
    if (auto_player) {
        if (cardsOnTable.empty()) card = getRandomCard(hand);
        else {
            char lastColor = cardsOnTable.back();
            auto it = std::find(hand.begin(), hand.end(), lastColor);
            if (it == hand.end()) card = getRandomCard(hand);
            else {
                it--;
                if (*it == '0') it--;
                if (*it == '1') {
                    card = hand.substr(it - hand.begin(), 3);
                    hand.erase(it - hand.begin(), 3);
                }
                else {
                    card = hand.substr(it - hand.begin(), 2);
                    hand.erase(it - hand.begin(), 2);
                }
            }
        }
        sleep(3);
    }
    else {
        std::cout << "Enter your card: ";
        std::cin >> card;
    }
    playedCard = card;
}
