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

void ClientProtocol::recvBUSY(int fd, std::string& taken) {
    std::string message = recvMessage(fd, -1);
    if (!tryParseBUSY(message, taken))
        throw Error("invalid BUSY message: " + message);
    logMessage(message, true);
}

void ClientProtocol::recvDEAL(int fd,
                              uint8_t& type,
                              std::string& first,
                              std::string& cards) {
    std::string message = recvMessage(fd, -1);
    if (!tryParseDEAL(message, type, first, cards))
        throw Error("invalid DEAL message: " + message);
    logMessage(message, true);
}

void ClientProtocol::recvTRICK(int fd,
                               uint8_t& trick,
                               std::string& cardsOnTable) {
    std::string message = recvMessage(fd, -1);
    if (!tryParseTRICK(message, trick, cardsOnTable))
        throw Error("invalid TRICK message: " + message);
    logMessage(message, true);
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
    std::regex re("^DEAL([1-7])([NESW])((?:(10|[2-9JQKA])[SHDC]){13})\r\n$");
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
    std::regex re("^TRICK(1[0-3]|[1-9])((?:(10|[2-9JQKA])[SHDC]){0,52})\r\n$");
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

bool tryParseTAKEN(std::string message);
bool tryParseSCORE(std::string message);
bool tryParseTOTAL(std::string message);

bool ClientProtocol::tryParseInputTRICK(std::string& input, std::string& card) {
    std::smatch match;
    std::regex re("^!((10|[2-9JQKA])[SHDC])$");
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
