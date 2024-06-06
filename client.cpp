#include "client.hpp"
#include "common.hpp"
#include "error.hpp"
#include "network_common.hpp"
#include "protocol_client.hpp"
#include <iostream>
#include <poll.h>

Client::Client(ClientConfig& config)
    : networker(config.host, config.port, config.domain),
      protocol(&networker, config.auto_player), seat(config.seat) {}

void Client::handleServer() {
    uint8_t type;
    std::string busy, first, cardsTaken, highestPlayer;
    std::map<std::string, unsigned int> scores, totals;
    std::string message = recvMessage(networker.sock_fd, -1);
    protocol.logMessage(message, true);
    lastTotal = false;

    if (protocol.tryParseBUSY(message, busy)) {
        protocol.displayBUSY(busy);
    }
    else if (protocol.tryParseDEAL(message, type, first, hand)) {
        protocol.displayDEAL(type, first, hand);
        lastPlayedCard.clear();
        lastPlayedTrick = 0;
        serverTrick = 0;
        tricksTaken.clear();
    }
    else if (protocol.tryParseTRICK(message, serverTrick, cardsOnTable)) {
        protocol.displayTRICK(serverTrick, cardsOnTable, hand);
        if (protocol.auto_player && serverTrick != lastPlayedTrick) {
            auto card = getAutoCard();
            deleteCard(hand, card);
            lastPlayedCard = card;
            lastPlayedTrick = serverTrick;
            protocol.sendTRICK(networker.sock_fd, serverTrick, card);
        }
    }
    else if (protocol.tryParseWRONG(message, serverTrick)) {
        protocol.displayWRONG(serverTrick);
        if (!lastPlayedCard.empty()) {
            hand.append(lastPlayedCard);
            lastPlayedCard.clear();
            lastPlayedTrick--;
        }
    }
    else if (protocol.tryParseTAKEN(message,
                                    serverTrick,
                                    cardsTaken,
                                    highestPlayer))
    {
        protocol.displayTAKEN(serverTrick, cardsTaken, highestPlayer);
        if (highestPlayer == seat) {
            hand += cardsTaken;
            tricksTaken.push_back(cardsTaken);
        }
    }
    else if (protocol.tryParseSCORE(message, scores)) {
        protocol.displaySCORE(scores);
    }
    else if (protocol.tryParseTOTAL(message, totals)) {
        protocol.displaySCORE(totals);
        lastTotal = true;
    }
}

void Client::handleInput() {
    std::string input, card;
    std::cin >> input;
    if (protocol.tryParseInputTRICK(input, card)) {
        if (lastPlayedTrick == serverTrick) {
            std::cout << "You already played a card in trick "
                      << std::to_string(serverTrick) + "." << std::endl;
        }
        else {
            deleteCard(hand, card);
            lastPlayedCard = card;
            lastPlayedTrick = serverTrick;
            protocol.sendTRICK(networker.sock_fd, serverTrick, card);
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

bool Client::connectToGame() {
    protocol.sendIAM(networker.sock_fd, seat);

    struct pollfd poll_fd[2];
    poll_fd[0].fd = networker.sock_fd;
    poll_fd[0].events = POLLIN;
    poll_fd[1].fd = protocol.auto_player ? -1 : STDIN_FILENO;
    poll_fd[1].events = POLLIN;

    try {
        while (true) {
            if (poll(poll_fd, 2, -1) < 0) throw Error("poll");
            if (poll_fd[0].revents & POLLIN) handleServer();
            else if (poll_fd[1].revents & POLLIN) handleInput();
            else throw Error("poll revents (server/STDIN)");
        }
    }
    catch (Error& e) {
        std::string msg = e.what();
        if (isSubstring(msg, "connection closed by peer") && lastTotal) {
            return true;
        }
        else {
            std::cerr << msg << std::endl;
            return false;
        }
    }
}

std::string Client::getAutoCard() {
    std::string card;
    if (!cardsOnTable.empty()) {
        card = findCardWithSuit(hand, cardsOnTable.back());
    }
    if (card.empty()) {
        card = getRandomCard(hand);
    }
    return card;
}
