#include "protocol_server.hpp"
#include "common.hpp"
#include "error.hpp"
#include "network_common.hpp"
#include <condition_variable>
#include <iostream>
#include <regex>
#include <signal.h>

ServerProtocol::ServerProtocol(ServerNetworker* networker, unsigned int timeout)
    : networker(networker), timeout(timeout) {
    signal(SIGPIPE, SIG_IGN);
}

void ServerProtocol::logMessage(int client_fd,
                                std::string message,
                                bool incoming) {
    auto clientInfo = networker->getClientInfo(client_fd);
    std::string from = clientInfo.first;
    std::string to = clientInfo.second;
    if (incoming) std::swap(from, to);
    std::cout << createLog(from, to, message) << std::flush;
}

void ServerProtocol::recvIAM(int fd, std::string& seat) {
    auto message = recvMessage(fd, timeout);
    std::regex pattern("^IAM[NESW]\r\n$");
    if (!std::regex_match(message, pattern))
        throw Error("invalid IAM message: " + message);
    logMessage(fd, message, true);
    seat = message.substr(3, 1);
}

void ServerProtocol::sendBUSY(int fd, std::string taken) {
    std::string message = "BUSY" + taken + "\r\n";
    sendMessage(fd, message);
    logMessage(fd, message, false);
}

void ServerProtocol::sendDEAL(int fd,
                              uint8_t type,
                              std::string first,
                              std::string cards) {
    std::string message =
        "DEAL" + std::to_string(type) + first + cards + "\r\n";
    sendMessage(fd, message);
    logMessage(fd, message, false);
}

void ServerProtocol::sendTRICK(int fd,
                               uint8_t trick,
                               std::string cardsOnTable) {
    std::string message =
        "TRICK" + std::to_string(trick) + cardsOnTable + "\r\n";
    sendMessage(fd, message);
    logMessage(fd, message, false);
}

void ServerProtocol::recvTRICK(int fd,
                               uint8_t* trick,
                               std::string& cardPlaced) {
    auto message = recvMessage(fd, -1);
    std::regex pattern("^TRICK[1-7]((10|[2-9JQKA])[SHDC])\r\n$");
    if (!std::regex_match(message, pattern))
        throw Error("invalid TRICK message: " + message);
    logMessage(fd, message, true);
    *trick = message[5] - '0';
    cardPlaced = message.substr(6, message.size() - 8);
}

void ServerProtocol::sendWRONG(int fd, uint8_t trick) {
    std::string message = "WRONG" + std::to_string(trick) + "\r\n";
    sendMessage(fd, message);
    logMessage(fd, message, false);
}
