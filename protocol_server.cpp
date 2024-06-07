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
                                const std::string& message,
                                bool incoming) {
    auto clientInfo = networker->getClientInfo(client_fd);
    std::string from = clientInfo.first;
    std::string to = clientInfo.second;
    if (incoming) std::swap(from, to);
    std::cout << createLog(from, to, message) << std::flush;
}

void ServerProtocol::recvIAM(int fd, std::string& seat) {
    auto message = recvMessage(fd, timeout);
    logMessage(fd, message, true);
    if (!tryParseIAM(message, seat)) throw Error("invalid IAM message");
}

void ServerProtocol::sendBUSY(int fd,
                              const std::string& busySeats,
                              std::unique_lock<std::mutex>* lock) {
    std::string message = "BUSY" + busySeats + "\r\n";
    sendMessage(fd, message, lock);
    logMessage(fd, message, false);
}

void ServerProtocol::sendDEAL(int fd,
                              uint8_t type,
                              const std::string& first,
                              const std::string& cards,
                              std::unique_lock<std::mutex>* lock) {
    std::string message =
        "DEAL" + std::to_string(type) + first + cards + "\r\n";
    sendMessage(fd, message, lock);
    logMessage(fd, message, false);
}

void ServerProtocol::sendTRICK(int fd,
                               uint8_t trick,
                               const std::string& cardsOnTable,
                               std::unique_lock<std::mutex>* lock) {
    std::string message =
        "TRICK" + std::to_string(trick) + cardsOnTable + "\r\n";
    sendMessage(fd, message, lock);
    logMessage(fd, message, false);
}

void ServerProtocol::recvTRICK(int fd,
                               uint8_t& trick,
                               std::string& cardPlaced) {
    auto message = recvMessage(fd, -1);
    logMessage(fd, message, true);
    if (!tryParseTRICK(message, trick, cardPlaced))
        throw Error("invalid TRICK message");
}

void ServerProtocol::sendWRONG(int fd,
                               uint8_t trick,
                               std::unique_lock<std::mutex>* lock) {
    std::string message = "WRONG" + std::to_string(trick) + "\r\n";
    sendMessage(fd, message, lock);
    logMessage(fd, message, false);
}

void ServerProtocol::sendTAKEN(int fd,
                               uint8_t trick,
                               const std::string& cardsTaken,
                               const std::string& seat,
                               std::unique_lock<std::mutex>* lock) {
    std::string message =
        "TAKEN" + std::to_string(trick) + cardsTaken + seat + "\r\n";
    sendMessage(fd, message, lock);
    logMessage(fd, message, false);
}

void ServerProtocol::sendSCORE(int fd,
                               std::map<std::string, unsigned int>& scores,
                               std::unique_lock<std::mutex>* lock) {
    std::string message = "SCORE";
    for (auto& [seat, score] : scores) {
        message += seat + std::to_string(score);
    }
    message += "\r\n";
    sendMessage(fd, message, lock);
    logMessage(fd, message, false);
}

void ServerProtocol::sendTOTAL(int fd,
                               std::map<std::string, unsigned int>& totals,
                               std::unique_lock<std::mutex>* lock) {
    std::string message = "TOTAL";
    for (auto& [seat, total] : totals) {
        message += seat + std::to_string(total);
    }
    message += "\r\n";
    sendMessage(fd, message, lock);
    logMessage(fd, message, false);
}

bool ServerProtocol::tryParseIAM(const std::string& message,
                                 std::string& seat) {
    std::smatch match;
    std::regex re("^IAM([NESW])\r\n$");
    if (std::regex_match(message, match, re)) {
        seat = match[1];
        return true;
    }
    return false;
}

bool ServerProtocol::tryParseTRICK(const std::string& message,
                                   uint8_t& trick,
                                   std::string& cardPlaced) {
    std::smatch match;
    std::regex re("^TRICK(1[0-3]|[1-9])((?:10|[2-9JQKA])[SHDC])\r\n$");
    if (std::regex_match(message, match, re)) {
        trick = std::stoi(match[1]);
        cardPlaced = match[2];
        return true;
    }
    return false;
}
