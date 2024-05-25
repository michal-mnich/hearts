#include "client.hpp"
#include "error.hpp"
#include "network.hpp"
#include <iostream>
#include <signal.h>

Client::Client(std::string host, std::string port, int domain)
    : networker(host, port, domain) {}

void Client::connectToGame() {
    signal(SIGPIPE, SIG_IGN);

    while (write(networker.sock_fd, "hello", 5) > 0) {
        sleep(1);
    }

    debug("Server disconnected");
}
