#include "client.hpp"
#include "common.hpp"
#include "error.hpp"
#include "protocol_client.hpp"
#include <iostream>

Client::Client(ClientConfig& config)
    : networker(config.host, config.port, config.domain), protocol(&networker),
      seat(config.seat), auto_player(config.auto_player) {}

void Client::connectToGame() {
    try {
        int fd = networker.sock_fd;

        protocol.sendIAM(fd, seat);

        uint8_t type;
        std::string first, cards;
        protocol.recvDEAL(fd, type, first, cards);
        uint8_t trick;
        std::string cardsOnTable;
        while (true) {
            protocol.recvTRICK(fd, &trick, cardsOnTable);
            std::cout << "Trick " << (int)trick << ": " << cardsOnTable
                      << std::endl;
            std::cout << "Enter your card: ";
            std::string card;
            std::cin >> card;
            protocol.sendTRICK(fd, trick, card);
        }
    }
    catch (Error& e) {
        std::cerr << e.what() << std::endl;
    }
}
