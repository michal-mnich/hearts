#include "client.hpp"
#include "error.hpp"
#include "protocol.hpp"
#include <iostream>
#include <signal.h>

Client::Client(std::string host, std::string port, int domain)
    : networker(host, port, domain) {}

void Client::connectToGame() {
    try {
        signal(SIGPIPE, SIG_IGN);
        while (true) {
            auto seat = std::string(1, 'a' + rand() % 26);
            seat = "n";
            protocol.sendIAM(networker.sock_fd, seat);
            debug("Sent IAM: " + seat);
            sleep(2);
        }
    }
    catch (std::exception& e) {
        debug(e.what());
    }
}
