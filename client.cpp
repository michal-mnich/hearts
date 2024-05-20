#include "client.hpp"
#include "network.hpp"
#include "error.hpp"
#include <iostream>
#include <signal.h>

Client::Client(const std::string& host, int port, int domain)
    : networker(host, port, domain) {}

void Client::connectToGame() {
    signal(SIGPIPE, SIG_IGN);

    networker.connectToServer();

    debug("Client (local) " + getLocalAddress(networker.getSocket()));
    debug("Server (foreign) " + getPeerAddress(networker.getSocket()));

    while (write(networker.getSocket(), "hello", 5) > 0) {
        debug("Sent message to server");
        sleep(1);
    }

    debug("Server disconnected, shutting down client...");
    closeSocket(networker.getSocket());
}
