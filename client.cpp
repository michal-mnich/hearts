#include "client.hpp"
#include "common.hpp"
#include "error.hpp"
#include "protocol_client.hpp"
#include <iostream>
#include <signal.h>

Client::Client(ClientConfig& config)
    : networker(config.host, config.port, config.domain), protocol(&networker),
      seat(config.seat), auto_player(config.auto_player) {}

void Client::connectToGame() {
    try {
        signal(SIGPIPE, SIG_IGN);
        protocol.sendIAM(networker.sock_fd, seat);

        uint8_t type;
        std::string first, cards;
        protocol.recvDEAL(networker.sock_fd, type, first, cards);
    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}
