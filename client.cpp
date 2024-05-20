#include "client.hpp"
#include "network.hpp"
#include <iostream>
#include <signal.h>

Client::Client(const std::string& host, int port, int domain)
    : networker(host, port, domain) {}

void Client::connectToGame() {
    signal(SIGPIPE, SIG_IGN);

    networker.connectToServer();

    std::cout << "server " << getPeerAddress(networker.getSocket()) << '\n';
    std::cout << "client " << getLocalAddress(networker.getSocket()) << '\n';

    while (write(networker.getSocket(), "hello", 5) > 0) {
        std::cout << "sent message to server" << std::endl;
        sleep(1);
    }

    std::cout << "Connection closed." << std::endl;
    closeSocket(networker.getSocket());
}
