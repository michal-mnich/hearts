#include "protocol_server.hpp"
#include "common.hpp"
#include "error.hpp"
#include "network_common.hpp"
#include <regex>

ServerProtocol::ServerProtocol(ServerNetworker* networker, unsigned int timeout)
    : networker(networker), timeout(timeout) {
    logFile.open(createFilename("server"), std::ios::out | std::ios::trunc);
}

ServerProtocol::~ServerProtocol() {
    logFile.close();
}

void ServerProtocol::logMessage(int client_fd,
                                std::string message,
                                bool incoming) {
    auto clientInfo = networker->getClientInfo(client_fd);
    std::string from = clientInfo.first;
    std::string to = clientInfo.second;
    if (incoming) std::swap(from, to);
    logFile << createLog(from, to, message);
}

// Returns the seat of the player
std::string ServerProtocol::recvIAM(int fd) {
    static char buffer[6] = {0};
    socket_set_timeout(fd, timeout);
    readn(fd, buffer, sizeof(buffer));
    socket_clear_timeout(fd);
    std::string message(buffer, 6);
    std::regex pattern("^IAM[NESW]\r\n$");
    if (!std::regex_match(message, pattern)) throw Error("invalid IAM message");
    logMessage(fd, message, true);
    debug("Received IAM" + message.substr(3, 1));
    return message.substr(3, 1);
}

void ServerProtocol::sendBUSY(int fd, std::string taken) {
    std::string message = "BUSY" + taken + "\r\n";
    writen(fd, message.c_str(), message.size());
    logMessage(fd, message, false);
    debug("Sent BUSY" + taken);
}
