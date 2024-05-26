#include "protocol_client.hpp"
#include "error.hpp"
#include "network_common.hpp"
#include "common.hpp"
#include <regex>

ClientProtocol::ClientProtocol(ClientNetworker* networker)
    : networker(networker) {
    logFile.open("client.log", std::ios::out | std::ios::trunc);
}

ClientProtocol::~ClientProtocol() {
    logFile.close();
}

void ClientProtocol::logMessage(std::string message, bool incoming) {
    std::string from = networker->localAddress;
    std::string to = networker->peerAddress;
    if (incoming) std::swap(from, to);
    logFile << createLog(from, to, message);
}

void ClientProtocol::sendIAM(int fd, std::string seat) {
    std::string message = "IAM" + seat + "\r\n";
    writen(fd, message.c_str(), message.size());
    logMessage(message, false);
}
