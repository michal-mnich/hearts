#include "protocol_client.hpp"
#include "common.hpp"
#include "error.hpp"
#include "network_common.hpp"
#include <iostream>
#include <regex>
#include <signal.h>

ClientProtocol::ClientProtocol(ClientNetworker* networker)
    : networker(networker) {
    signal(SIGPIPE, SIG_IGN);
}

void ClientProtocol::logMessage(std::string message, bool incoming) {
    std::string from = networker->localAddress;
    std::string to = networker->peerAddress;
    if (incoming) std::swap(from, to);
    std::cout << createLog(from, to, message) << std::flush;
}

void ClientProtocol::sendIAM(int fd, std::string seat) {
    std::string message = "IAM" + seat + "\r\n";
    sendMessage(fd, message);
    logMessage(message, false);
}

void ClientProtocol::recvBUSY(int fd, std::string& taken) {
    std::string message = recvMessage(fd, -1);
    std::regex pattern("^BUSY[NESW]{1,4}\r\n$");
    if (!std::regex_match(message, pattern))
        throw Error("invalid BUSY message: " + message);
    logMessage(message, true);
    int num_taken = message.size() - 6;
    taken = message.substr(4, num_taken);
}

void ClientProtocol::recvDEAL(int fd,
                              uint8_t& type,
                              std::string& first,
                              std::string& cards) {
    std::string message = recvMessage(fd, -1);
    std::regex pattern("^DEAL[1-7][NESW]((?:(10|[2-9JQKA])[SHDC]){13})\r\n$");
    if (!std::regex_match(message, pattern))
        throw Error("invalid DEAL message: " + message);
    logMessage(message, true);
    type = message[4] - '0';
    first = message[5];
    cards = message.substr(6, 26);
}

void ClientProtocol::recvTRICK(int fd, uint8_t* trick, std::string& cardsOnTable) {
    std::string message = recvMessage(fd, -1);
    std::regex pattern("^TRICK[1-7]((?:(10|[2-9JQKA])[SHDC]){0,50})\r\n$");
    if (!std::regex_match(message, pattern))
        throw Error("invalid TRICK message: " + message);
    logMessage(message, true);
    *trick = message[5] - '0';
    cardsOnTable = message.substr(6, message.size() - 8);
}

void ClientProtocol::sendTRICK(int fd, uint8_t trick, std::string cardPlaced) {
    std::string message = "TRICK" + std::to_string(trick) + cardPlaced + "\r\n";
    sendMessage(fd, message);
    logMessage(message, false);
}
