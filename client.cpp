#include "client.hpp"
#include "error.hpp"
#include "protocol_client.hpp"
#include <iostream>
#include <signal.h>

Client::Client(std::string host, std::string port, int domain)
    : networker(host, port, domain), protocol(&networker) {}

void Client::connectToGame() {
    try {
        signal(SIGPIPE, SIG_IGN);
        while (true) {
            std::string seat = "N";
            protocol.sendIAM(networker.sock_fd, seat);
            debug("Sent IAM: " + seat);
            sleep(2);
        }
    }
    catch (std::exception& e) {
        debug(e.what());
    }
}
