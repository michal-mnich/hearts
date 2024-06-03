#include "protocol_client.hpp"
#include "common.hpp"
#include "error.hpp"
#include "network_common.hpp"
#include <iostream>
#include <regex>
#include <signal.h>

ClientProtocol::ClientProtocol(ClientNetworker* networker, bool auto_player)
    : networker(networker), auto_player(auto_player) {
    signal(SIGPIPE, SIG_IGN);
}

void ClientProtocol::logMessage(std::string message, bool incoming) {
    if (auto_player) {
        std::string from = networker->localAddress;
        std::string to = networker->peerAddress;
        if (incoming) std::swap(from, to);
        std::cout << createLog(from, to, message) << std::flush;
    }
}

void ClientProtocol::sendIAM(int fd, std::string seat) {
    std::string message = "IAM" + seat + "\r\n";
    sendMessage(fd, message);
    logMessage(message, false);
}

void ClientProtocol::sendTRICK(int fd, uint8_t trick, std::string cardPlaced) {
    std::string message = "TRICK" + std::to_string(trick) + cardPlaced + "\r\n";
    sendMessage(fd, message);
    logMessage(message, false);
}

bool ClientProtocol::tryParseBUSY(const std::string& message,
                                  std::string& taken) {
    std::smatch match;
    std::regex re("^BUSY([NESW]{1,4})\r\n$");
    if (std::regex_match(message, match, re)) {
        taken = match[1];
        return true;
    }
    return false;
}

bool ClientProtocol::tryParseDEAL(const std::string& message,
                                  uint8_t& type,
                                  std::string& first,
                                  std::string& cards) {
    std::smatch match;
    std::regex re("^DEAL([1-7])([NESW])((?:(?:10|[2-9JQKA])[SHDC]){13})\r\n$");
    if (std::regex_match(message, match, re)) {
        type = std::stoi(match[1]);
        first = match[2];
        cards = match[3];
        return true;
    }
    return false;
}

bool ClientProtocol::tryParseTRICK(const std::string& message,
                                   uint8_t& trick,
                                   std::string& cardsOnTable) {
    std::smatch match;
    std::regex re("^TRICK(1[0-3]|[1-9])((?:(?:10|[2-9JQKA])[SHDC]){0,3})\r\n$");
    if (std::regex_match(message, match, re)) {
        trick = std::stoi(match[1]);
        cardsOnTable = match[2];
        return true;
    }
    return false;
}

bool ClientProtocol::tryParseWRONG(const std::string& message, uint8_t& trick) {
    std::smatch match;
    std::regex re("^WRONG(1[0-3]|[1-9])\r\n$");
    if (std::regex_match(message, match, re)) {
        trick = std::stoi(match[1]);
        return true;
    }
    return false;
}

bool ClientProtocol::tryParseTAKEN(std::string message,
                                   uint8_t& trick,
                                   std::string& cardsTaken,
                                   std::string& seat) {
    std::smatch match;
    std::regex re(
        "^TAKEN(1[0-3]|[1-9])((?:(?:10|[2-9JQKA])[SHDC]){4})([NESW])\r\n$");
    if (std::regex_match(message, match, re)) {
        trick = std::stoi(match[1]);
        cardsTaken = match[2];
        seat = match[3];
        return true;
    }
    return false;
}

static std::string getScoresGroup() {
    std::string seatGroup = "([NESW])";
    std::string scoreGroup = "([0-9]+)";
    std::string groups;
    for (int i = 0; i < 4; i++) {
        groups += seatGroup + scoreGroup;
    }
    return groups;
}

static std::map<std::string, unsigned int> getScoresMap(std::smatch& match) {
    std::map<std::string, unsigned int> scores;
    for (int i = 0; i < 4; i++) {
        std::string seat = match[1 + 2 * i];
        unsigned int score = std::stoi(match[2 + 2 * i]);
        scores[seat] = score;
    }
    return scores;
}

bool ClientProtocol::tryParseSCORE(
    std::string message,
    std::map<std::string, unsigned int>& scores) {
    std::smatch match;
    std::regex re("^SCORE" + getScoresGroup() + "\r\n$");
    if (std::regex_match(message, match, re)) {
        scores = getScoresMap(match);
        return true;
    }
    return false;
}

bool ClientProtocol::tryParseTOTAL(
    std::string message,
    std::map<std::string, unsigned int>& totals) {
    std::smatch match;
    std::regex re("^TOTAL" + getScoresGroup() + "\r\n$");
    if (std::regex_match(message, match, re)) {
        totals = getScoresMap(match);
        return true;
    }
    return false;
}

bool ClientProtocol::tryParseInputTRICK(std::string& input, std::string& card) {
    std::smatch match;
    std::regex re("^!((?:10|[2-9JQKA])[SHDC])$");
    if (std::regex_match(input, match, re)) {
        card = match[1];
        return true;
    }
    return false;
}

bool ClientProtocol::tryParseInputCards(std::string& input) {
    std::regex re("^cards$");
    return std::regex_match(input, re);
}

bool ClientProtocol::tryParseInputTricks(std::string& input) {
    std::regex re("^tricks$");
    return std::regex_match(input, re);
}

void ClientProtocol::displayBUSY(std::string& taken) {
    if (auto_player) return;
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

void ClientProtocol::displayDEAL(uint8_t type,
                                 std::string& first,
                                 std::string& hand) {
    if (auto_player) return;
    std::cout << "New deal " << std::to_string(type) << ": staring place "
              << first << ", your cards: " << getPrettyCards(hand, true) << "."
              << std::endl;
}

void ClientProtocol::displayWRONG(uint8_t trick) {
    if (auto_player) return;
    std::cout << "Wrong message received in trick " << std::to_string(trick)
              << "." << std::endl;
}

void ClientProtocol::displayTAKEN(uint8_t trick,
                                  std::string& cardsTaken,
                                  std::string& takingPlayer) {
    if (auto_player) return;
    std::cout << "A trick " << std::to_string(trick) << " is taken by "
              << takingPlayer << ", cards " << getPrettyCards(cardsTaken, true)
              << "." << std::endl;
}

void ClientProtocol::displaySCORE(std::map<std::string, unsigned int>& scores) {
    if (auto_player) return;
    std::cout << "The total scores are:" << std::endl;
    for (const auto& [seat, score] : scores) {
        std::cout << seat << " | " << std::to_string(score) << std::endl;
    }
}

void ClientProtocol::displayTRICK(uint8_t trick,
                                  std::string& cardsOnTable,
                                  std::string& hand) {
    if (auto_player) return;
    std::cout << "Trick: (" << std::to_string(trick) << ") "
              << getPrettyCards(cardsOnTable) << std::endl;
    std::cout << "Available: " << getPrettyCards(hand, true) << std::endl;
}
