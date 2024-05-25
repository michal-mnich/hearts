#include "server.hpp"
#include "error.hpp"
#include "network.hpp"
#include <iostream>
#include <poll.h>

Server::Server(int port)
    : networker(port), active_clients(0), game_over(false) {}

void Server::start() {
    networker.createSocket();
    networker.bindSocket();
    networker.listenSocket();
    debug("Server (local IPv4) " + getLocalAddress(networker.getSocket()));

    accept_thread = std::thread(&Server::acceptThread, this);

    debug("Starting game...");
    game_thread = std::thread(&Server::gameThread, this);

    accept_thread.join();
    game_thread.join();

    debug("Game over, waiting for clients to disconnect...");

    std::unique_lock<std::mutex> lock(mtx);
    active_clients_cv.wait(lock, [this] { return active_clients == 0; });

    debug("All clients disconnected, server shutting down");

    closeSocket(networker.getSocket());
    for (int client_socket : client_sockets) {
        closeSocket(client_socket);
    }
}

void Server::handleGameOver() {
    game_over.store(true);

    shutdownSocket(networker.getSocket());

    std::lock_guard<std::mutex> lock(mtx);
    for (int client_socket : client_sockets) {
        shutdownSocket(client_socket);
    }
}

void Server::acceptThread() {
    struct pollfd fds[1];
    fds[0].fd = networker.getSocket();
    fds[0].events = POLLIN;

    while (true) {
        if (poll(fds, 1, -1) < 0) throw new SystemError("poll");

        if (game_over.load()) {
            debug("Game over, exiting accept thread");
            break;
        }

        auto fd = fds[0];

        if (fd.revents & POLLIN) {
            int client_socket = accept(fd.fd, nullptr, nullptr);
            if (client_socket < 0) throw new SystemError("accept");
            handleNewClient(client_socket);
        }
        if (fd.revents & POLLHUP || fd.revents & POLLERR)
            throw new SystemError("revents");
    }
}

void Server::clientThread(int client_socket) {
    {
        std::lock_guard<std::mutex> lock(mtx);
        active_clients++;
    }

    // Handle client communication
    char buffer[1024] = {0};
    while (true) {
        int bytes_read = read(client_socket, buffer, sizeof(buffer));
        if (game_over.load()) {
            debug("Game over, exiting client thread");
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
    debug("Game over");
    handleGameOver();
}

void Server::handleNewClient(int client_socket) {
    std::lock_guard<std::mutex> lock(mtx);
    if (client_sockets.size() == 4) {
        // game is full
        debug("Game is full, rejecting client");
        closeSocket(client_socket);
        return;
    }
    debug("Client (foreign) " + getPeerAddress(client_socket));
    client_sockets.insert(client_socket);
    std::thread(&Server::clientThread, this, client_socket).detach();
}
