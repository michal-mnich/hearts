#include "server.hpp"
#include "error.hpp"
#include <iostream>
#include <poll.h>

Server::Server(uint16_t port)
    : networker(port), active_clients(0), game_over(false) {}

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
    debug("Client threads finished");
}

void Server::handleGameOver() {
    debug("Game over!!!");
    game_over.store(true);
    networker.stopAccepting();
    networker.disconnectClients();
}

void Server::acceptThread() {
    networker.startAccepting(this);
    if (!game_over.load())
        throw SystemError("unexpectedly stopped accepting clients");
}

void Server::playerThread(int fd) {
    {
        std::lock_guard<std::mutex> lock(mtx);
        active_clients++;
    }

    // Handle client communication
    char buffer[1024] = {0};
    while (true) {
        int bytes_read = read(fd, buffer, sizeof(buffer));
        if (game_over.load()) {
            break;
        }
        if (bytes_read <= 0) {
            break;
        }
        // Process client data
    }

    {
        std::lock_guard<std::mutex> lock(mtx);
        active_clients--;
        if (active_clients == 0) active_clients_cv.notify_one();
    }
}

void Server::gameThread() {
    sleep(20);
    handleGameOver();
}
