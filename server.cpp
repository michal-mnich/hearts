#include "server.hpp"
#include "common.hpp"
#include "error.hpp"
#include "protocol.hpp"
#include <iostream>
#include <poll.h>

Server::Server(uint16_t port, unsigned int timeout)
    : networker(port), protocol(timeout), active_clients(0), game_over(false) {}

void Server::start() {
    debug("Starting accept thread...");
    accept_thread = std::thread(&Server::acceptThread, this);

    debug("Starting game thread...");
    game_thread = std::thread(&Server::gameThread, this);

    accept_thread.join();
    debug("Accept thread finished");

    game_thread.join();
    debug("Game thread finished");

    std::unique_lock<std::mutex> lock(mtx);
    active_clients_cv.wait(lock, [this] { return active_clients == 0; });
    debug("All client threads finished");
}

void Server::handleGameOver() {
    std::lock_guard<std::mutex> lock(mtx);
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
    {
        std::lock_guard<std::mutex> lock(mtx);
        active_clients++;
    }

    while (true) {
        try {
            auto seat = protocol.recvIAM(fd);
            debug("Received IAM: " + seat);
        }
        catch (Error& e) {
            auto msg = std::string(e.what());
            debug(msg);
            if (isSubstring(msg, "timeout") || isSubstring(msg, "invalid") ||
                game_over.load())
            {
                break;
            }
        }
    }

    {
        std::lock_guard<std::mutex> lock(mtx);
        networker.removeClient(fd);
        active_clients--;
        if (active_clients == 0) active_clients_cv.notify_one();
    }
}

void Server::gameThread() {
    sleep(20);
    handleGameOver();
}
