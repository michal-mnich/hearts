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
    debug("Sent IAM" + seat);
}

std::string ClientProtocol::recvBUSY(int fd) {
    std::string message = readUntilEnd(fd);
    std::regex pattern("^BUSY[NESW]{1,4}\r\n$");
    if (!std::regex_match(message, pattern))
        throw Error("invalid BUSY message");
    logMessage(message, true);
    int num_taken = message.size() - 6;
    std::string taken = message.substr(4, num_taken);
    debug("Received BUSY" + taken);
    return taken;
}
