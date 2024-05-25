#include "protocol.hpp"
#include "error.hpp"
#include "network_common.hpp"
#include <regex>

ServerProtocol::ServerProtocol(unsigned int timeout) : timeout(timeout) {}
ClientProtocol::ClientProtocol() {}

// Returns the seat of the player
std::string ServerProtocol::recvIAM(int fd) {
    static char buffer[6] = {0};
    socket_set_timeout(fd, timeout);
    readn(fd, buffer, sizeof(buffer));
    socket_clear_timeout(fd);
    std::string message(buffer, 6);
    std::regex pattern("IAM[nesw]\r\n");
    if (!std::regex_match(message, pattern)) throw Error("invalid IAM message");
    return message.substr(3, 1);
}

void ClientProtocol::sendIAM(int fd, std::string seat) {
    std::string message = "IAM" + seat + "\r\n";
    writen(fd, message.c_str(), message.size());
}
