#include "client.hpp"
#include "common.hpp"
#include "error.hpp"
#include "network_common.hpp"
#include "protocol_client.hpp"
#include <algorithm>
#include <iostream>
#include <poll.h>

void displayBUSY(std::string& taken) {
    std::string busy;
    for (size_t i = 0; i < taken.size(); i++) {
        busy += taken[i];
        if (i != taken.size() - 1) {
            busy += ", ";
        }
    }
    std::cout << "Place busy, list of busy places received: " << busy << "."
              << std::endl;
}

void displayDEAL(uint8_t type, std::string& first, std::string& hand) {
    std::cout << "New deal " << std::to_string(type) << ": staring place "
              << first << ", your cards: " << getPrettyCards(hand, true) << "."
              << std::endl;
}

void displayWRONG(uint8_t trick) {
    std::cout << "Wrong message received in trick " << std::to_string(trick)
              << "." << std::endl;
}

void displayTAKEN(uint8_t trick,
                  std::string& cardsTaken,
                  std::string& takingPlayer) {
    std::cout << "A trick " << std::to_string(trick) << " is taken by "
              << takingPlayer << ", cards " << getPrettyCards(cardsTaken, true)
              << "." << std::endl;
}

void displaySCORE(std::string& p1,
                  unsigned int s1,
                  std::string& p2,
                  unsigned int s2,
                  std::string& p3,
                  unsigned int s3,
                  std::string& p4,
                  unsigned int s4) {
    std::cout << "The total scores are:" << std::endl;
    std::cout << p1 << " | " << std::to_string(s1) << std::endl;
    std::cout << p2 << " | " << std::to_string(s2) << std::endl;
    std::cout << p3 << " | " << std::to_string(s3) << std::endl;
    std::cout << p4 << " | " << std::to_string(s4) << std::endl;
}

void displayTRICK(uint8_t trick, std::string& cardsOnTable, std::string& hand) {
    std::cout << "Trick: (" << std::to_string(trick) << ") "
              << getPrettyCards(cardsOnTable) << std::endl;
    std::cout << "Available: " << getPrettyCards(hand, true) << std::endl;
}

Client::Client(ClientConfig& config)
    : networker(config.host, config.port, config.domain),
      protocol(&networker, config.auto_player), seat(config.seat),
      auto_player(config.auto_player) {}

void Client::connectToGame() {
    std::string busy, first, cardsOnTable, highestPlayer, cardsTaken;
    uint8_t type, trick, lastPlayedTrick = -1;
    int fd = networker.sock_fd;

    protocol.sendIAM(fd, seat);

    struct pollfd poll_fd[2];
    poll_fd[0].fd = fd;
    poll_fd[0].events = POLLIN;
    poll_fd[1].fd = (auto_player) ? -1 : STDIN_FILENO;
    poll_fd[1].events = POLLIN;

    while (true) {
        if (poll(poll_fd, 2, -1) < 0) throw Error("poll");

        if (poll_fd[0].revents & POLLIN) {
            std::string message = recvMessage(fd, -1);

            if (protocol.tryParseBUSY(message, busy)) {
                protocol.logMessage(message, true);
                if (!auto_player) displayBUSY(busy);
            }
            else if (protocol.tryParseDEAL(message, type, first, hand)) {
                protocol.logMessage(message, true);
                if (!auto_player) displayDEAL(type, first, hand);
            }
            else if (protocol.tryParseTRICK(message, trick, cardsOnTable)) {
                protocol.logMessage(message, true);
                if (!auto_player) displayTRICK(trick, cardsOnTable, hand);
                if (auto_player && trick != lastPlayedTrick) {
                    auto card = getAutoCard(cardsOnTable);
                    deleteCard(hand, card);
                    lastPlayedCard = card;
                    lastPlayedTrick = trick;
                    protocol.sendTRICK(fd, trick, card);
                }
            }
            else if (protocol.tryParseWRONG(message, trick)) {
                protocol.logMessage(message, true);
                if (!auto_player) displayWRONG(trick);
                hand.append(lastPlayedCard);
                lastPlayedCard.clear();
                lastPlayedTrick = -1;
            }
            else if (protocol.tryParseTAKEN(message,
                                            trick,
                                            cardsTaken,
                                            highestPlayer))
            {
                protocol.logMessage(message, true);
                if (!auto_player) {
                    displayTAKEN(trick, cardsTaken, highestPlayer);
                }
                if (highestPlayer == seat) {
                    hand += cardsTaken;
                    tricksTaken.push_back(cardsTaken);
                }
            }
        }
        else if (poll_fd[1].revents & POLLIN) {
            std::string input, card;
            std::cin >> input;
            if (protocol.tryParseInputTRICK(input, card)) {
                if (lastPlayedTrick == trick) {
                    std::cout << "You already played a card in trick "
                              << std::to_string(trick) + "." << std::endl;
                }
                else {
                    deleteCard(hand, card);
                    lastPlayedCard = card;
                    lastPlayedTrick = trick;
                    protocol.sendTRICK(fd, trick, card);
                }
            }
            else if (protocol.tryParseInputCards(input)) {
                std::cout << getPrettyCards(hand, true) << std::endl;
            }
            else if (protocol.tryParseInputTricks(input)) {
                for (auto trick : tricksTaken) {
                    std::cout << getPrettyCards(trick) << std::endl;
                }
            }
            else {
                std::cout << "Invalid input." << std::endl;
            }
        }
        else {
            throw Error("poll revents");
        }
    }
}

std::string Client::getAutoCard(const std::string& cardsOnTable) {
    std::string card;
    if (!cardsOnTable.empty()) {
        card = findCardWithSuit(hand, cardsOnTable.back());
    }
    if (card.empty()) {
        card = getRandomCard(hand);
    }
    return card;
}
