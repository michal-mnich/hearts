#include "server.hpp"
#include "common.hpp"
#include "error.hpp"
#include "protocol_server.hpp"
#include <iostream>
#include <poll.h>

Server::Server(uint16_t port, unsigned int timeout)
    : networker(port), protocol(&networker, timeout), game_over(false) {}

void Server::start() {
    debug("Starting accept thread...");
    accept_thread = std::thread(&Server::acceptThread, this);

    debug("Starting game thread...");
    game_thread = std::thread(&Server::gameThread, this);

    game_thread.join();

    debug("Game over!");
    game_over.store(true);

    networker.stopAccepting();
    accept_thread.join();
    debug("Accept thread stopped!");

    networker.disconnectAll();
    networker.joinClients();
    debug("All clients threads stopped!");
}

void Server::acceptThread() {
    networker.startAccepting(this);
    if (!game_over.load())
        throw Error("unexpectedly stopped accepting clients");
}

void Server::playerThread(int fd) {
    try {
        handleIAM(fd);
    }
    catch (Error& e) {
        std::cerr << e.what() << std::endl;
        goto exit;
    }

    while (true)
        ;

exit:
    networker.removeClient(fd);
}

void Server::gameThread() {
    while (true)
        ;
}

void Server::handleIAM(int fd) {
    auto seat = protocol.recvIAM(fd);

    std::lock_guard<std::mutex> lock(mtx);
    if (players.contains(seat)) {
        protocol.sendBUSY(fd, getKeys(players));
        throw Error("seat " + seat + " already taken");
    }
    else {
        players[seat] = fd;
        debug("Seat " + seat + " taken by client " +
              networker.getClientInfo(fd).second);
    }
}
