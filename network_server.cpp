#include "network_server.hpp"
#include "error.hpp"
#include "network_common.hpp"
#include "server.hpp"
#include <cstring>
#include <poll.h>

ServerNetworker::ServerNetworker(uint16_t port) {
    ipv4_fd = _socket(AF_INET);
    ipv6_fd = _socket(AF_INET6);

    std::memset(&ipv4_addr, 0, sizeof(ipv4_addr));
    std::memset(&ipv6_addr, 0, sizeof(ipv6_addr));

    ipv4_addr.sin_family = AF_INET;
    ipv4_addr.sin_addr.s_addr = INADDR_ANY;
    ipv4_addr.sin_port = htons(port);

    ipv6_addr.sin6_family = AF_INET6;
    ipv6_addr.sin6_addr = in6addr_any;
    ipv6_addr.sin6_port = htons(port);

    _bind(ipv4_fd, (struct sockaddr*)&ipv4_addr, sizeof(ipv4_addr));
    _bind(ipv6_fd, (struct sockaddr*)&ipv6_addr, sizeof(ipv6_addr));

    _listen(ipv4_fd);
    _listen(ipv6_fd);

    debug("IPv4 server at: " + getLocalAddress(ipv4_fd));
    debug("IPv6 server at: " + getLocalAddress(ipv6_fd));
}

void ServerNetworker::startAccepting(Server* server) {
    struct pollfd fds[2];
    fds[0].fd = ipv4_fd;
    fds[0].events = POLLIN;
    fds[1].fd = ipv6_fd;
    fds[1].events = POLLIN;

    while (true) {
        if (poll(fds, 2, -1) < 0) return;
        for (auto fd : fds) {
            if (fd.revents & POLLIN) {
                int accepted_fd = _accept(fd.fd);
                if (clients.size() >= MAX_CLIENTS) {
                    debug("Max clients reached, disconnecting new client");
                    _close(accepted_fd);
                }
                else {
                    clients[accepted_fd] = {getLocalAddress(accepted_fd),
                                            getPeerAddress(accepted_fd)};
                    debug(clients[accepted_fd].first +
                          " accepted connection from " +
                          clients[accepted_fd].second);
                    std::thread(&Server::playerThread, server, accepted_fd)
                        .detach();
                }
            }
            else if (fd.revents & POLLHUP || fd.revents & POLLERR) return;
        }
    }
}

void ServerNetworker::stopAccepting() {
    debug("Shutting down listening sockets...");
    _shutdown(ipv4_fd, SHUT_RDWR);
    _shutdown(ipv6_fd, SHUT_RDWR);
}

void ServerNetworker::disconnectClients() {
    debug("Shutting down client sockets...");
    for (auto c : clients)
        _shutdown(c.first, SHUT_RDWR);
}

ServerNetworker::~ServerNetworker() {
    debug("Closing listening sockets...");
    _close(ipv4_fd);
    _close(ipv6_fd);
    debug("Closing client sockets...");
    for (auto c : clients)
        _close(c.first);
}
