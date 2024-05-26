#include "client.hpp"
#include "error.hpp"
#include "protocol_client.hpp"
#include "common.hpp"
#include <iostream>
#include <signal.h>

Client::Client(std::string host, std::string port, int domain)
    : networker(host, port, domain), protocol(&networker) {}

void Client::connectToGame() {
    try {
        signal(SIGPIPE, SIG_IGN);
        while (true) {
            auto seat = getRandomSeat();
            protocol.sendIAM(networker.sock_fd, seat);
            debug("Sent IAM: " + seat);
            sleep(2);
        }
    }
    catch (std::exception& e) {
        debug(e.what());
        auto taken = protocol.recvBUSY(networker.sock_fd);
    }
}
