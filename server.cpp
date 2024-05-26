#include "server.hpp"
#include "common.hpp"
#include "error.hpp"
#include "protocol_server.hpp"
#include <iostream>
#include <poll.h>

Server::Server(uint16_t port, unsigned int timeout)
    : networker(port), protocol(&networker, timeout),
      game_over(false) {}

void Server::start() {
    debug("Starting accept thread...");
    accept_thread = std::thread(&Server::acceptThread, this);

    debug("Starting game thread...");
    game_thread = std::thread(&Server::gameThread, this);

    accept_thread.join();
    debug("Accept thread finished");

    game_thread.join();
    debug("Game thread finished");

    networker.joinClients();
}

void Server::handleGameOver() {
    debug("Game over!!!");
    game_over.store(true);
    networker.stopAccepting();
    networker.disconnectAll();
}

void Server::acceptThread() {
    networker.startAccepting(this);
    if (!game_over.load())
        throw Error("unexpectedly stopped accepting clients");
}

void Server::playerThread(int fd) {
    while (true) {
        try {
            auto seat = protocol.recvIAM(fd);
            debug("Received IAM: " + seat);
        }
        catch (Error& e) {
            auto msg = std::string(e.what());
            debug(msg);
            break;
        }
    }

    networker.removeClient(fd);
}

void Server::gameThread() {
    sleep(20);
    handleGameOver();
}

// void Server::addPlayer
