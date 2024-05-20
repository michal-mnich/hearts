#include "server.hpp"
#include "error.hpp"
#include "network.hpp"
#include <iostream>
#include <poll.h>

Server::Server(int port)
    : ipv4_networker(port), ipv6_networker(port), active_clients(0),
      game_over(false) {}

void Server::start() {
    ipv4_networker.createSocket();
    ipv4_networker.bindSocket();
    ipv4_networker.listenSocket();
    std::cout << "IPv4 server " << getLocalAddress(ipv4_networker.getSocket())
              << "\n";

    ipv6_networker.createSocket();
    ipv6_networker.bindSocket();
    ipv6_networker.listenSocket();
    std::cout << "IPv6 server " << getLocalAddress(ipv6_networker.getSocket())
              << "\n";

    accept_thread = std::thread(&Server::acceptThread, this);

    std::cout << "Starting game...\n";
    game_thread = std::thread(&Server::gameThread, this);

    accept_thread.join();
    game_thread.join();

    std::cout << "Game over, waiting for clients to disconnect...\n";

    std::unique_lock<std::mutex> lock(mtx);
    active_clients_cv.wait(lock, [this] { return active_clients == 0; });

    std::cout << "All clients disconnected, server shutting down\n";

    closeSocket(ipv4_networker.getSocket());
    closeSocket(ipv6_networker.getSocket());
    for (int client_socket : client_sockets) {
        closeSocket(client_socket);
    }
}

void Server::handleGameOver() {
    game_over.store(true);

    shutdownSocket(ipv4_networker.getSocket());
    shutdownSocket(ipv6_networker.getSocket());

    std::lock_guard<std::mutex> lock(mtx);
    for (int client_socket : client_sockets) {
        shutdownSocket(client_socket);
    }
}

void Server::acceptThread() {
    struct pollfd fds[2];
    fds[0].fd = ipv4_networker.getSocket();
    fds[0].events = POLLIN;
    fds[1].fd = ipv6_networker.getSocket();
    fds[1].events = POLLIN;

    while (true) {
        if (poll(fds, 2, -1) < 0) throw new NetworkError("poll");

        std::cout << "Poll returned\n";

        if (game_over.load()) {
            std::cout << "Game over accept\n";
            break;
        }

        if (fds[0].revents & POLLIN) {
            struct sockaddr_in client_address;
            socklen_t client_address_len = sizeof(client_address);
            int client_socket = accept(fds[0].fd,
                                       (struct sockaddr*)&client_address,
                                       &client_address_len);
            if (client_socket <= 0) {
                std::cerr << "Failed to accept client IPv4 connection\n";
                break;
            }
            handleNewClient(client_socket);
        }

        if (fds[1].revents & POLLIN) {
            struct sockaddr_in6 client_address;
            socklen_t client_address_len = sizeof(client_address);
            int client_socket = accept(fds[1].fd,
                                       (struct sockaddr*)&client_address,
                                       &client_address_len);
            if (client_socket <= 0) {
                std::cerr << "Failed to accept client IPv6 connection\n";
                break;
            }
            handleNewClient(client_socket);
        }
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
            std::cout << "Game over client\n";
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
    sleep(5);
    std::cout << "game over\n";
    handleGameOver();
}

void Server::handleNewClient(int client_socket) {
    std::lock_guard<std::mutex> lock(mtx);
    if (client_sockets.size() == 4) {
        // game is full
        std::cerr << "Game is full, rejecting client\n";
        close(client_socket);
        return;
    }
    std::cout << "client " << getPeerAddress(client_socket) << " connected\n";
    client_sockets.insert(client_socket);
    std::thread(&Server::clientThread, this, client_socket).detach();
}
