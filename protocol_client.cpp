#include "protocol_client.hpp"
#include "common.hpp"
#include "error.hpp"
#include "network_common.hpp"
#include <iostream>
#include <regex>

ClientProtocol::ClientProtocol(ClientNetworker* networker)
    : networker(networker) {}

void ClientProtocol::logMessage(std::string message, bool incoming) {
    std::string from = networker->localAddress;
    std::string to = networker->peerAddress;
    if (incoming) std::swap(from, to);
    std::cout << createLog(from, to, message) << std::flush;
}

void ClientProtocol::sendIAM(int fd, std::string seat) {
    std::string message = "IAM" + seat + "\r\n";
    writen(fd, message.c_str(), message.size());
    logMessage(message, false);
}

void ClientProtocol::recvBUSY(int fd, std::string& taken) {
    std::string message = readUntilEnd(fd);
    std::regex pattern("^BUSY[NESW]{1,4}\r\n$");
    if (!std::regex_match(message, pattern))
        throw Error("invalid BUSY message");
    logMessage(message, true);
    int num_taken = message.size() - 6;
    taken = message.substr(4, num_taken);
}

void ClientProtocol::recvDEAL(int fd,
                              uint8_t& type,
                              std::string& first,
                              std::string& cards) {
    std::string message = readUntilEnd(fd);
    std::regex pattern("^DEAL[1-7][NESW]((?:(10|[2-9JQKA])[SHDC]){13})\r\n$");
    if (!std::regex_match(message, pattern))
        throw Error("invalid DEAL message");
    logMessage(message, true);
    type = message[4] - '0';
    first = message[5];
    cards = message.substr(6, 26);
}
