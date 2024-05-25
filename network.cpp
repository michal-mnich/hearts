#include "network.hpp"
#include "error.hpp"
#include "server.hpp"
#include <cstring>
#include <poll.h>

#define QUEUE_SIZE 4

static void _close(int fd) {
    if (close(fd) < 0) throw SystemError("close");
}

static void _shutdown(int fd, int how) {
    if (shutdown(fd, how) < 0) throw SystemError("shutdown");
}

static int _socket(int domain) {
    int opt, optname;

    int sock_fd = socket(domain, SOCK_STREAM, 0);
    if (sock_fd < 0) throw SystemError("socket");

    // Allow reuse of local addresses (no TIME_WAIT)
    opt = 1;
    optname = SO_REUSEADDR;
    if (setsockopt(sock_fd, SOL_SOCKET, optname, &opt, sizeof(opt)) < 0)
        throw SystemError("setsockopt (SO_REUSEADDR)");

    // Allow only IPv6 connections on an IPv6 socket
    if (domain == AF_INET6) {
        opt = 1;
        optname = IPV6_V6ONLY;
        if (setsockopt(sock_fd, IPPROTO_IPV6, optname, &opt, sizeof(opt)) < 0)
            throw SystemError("setsockopt (IPV6_V6ONLY)");
    }

    return sock_fd;
}

static void _bind(int sock_fd, struct sockaddr* addr, socklen_t len) {
    if (bind(sock_fd, addr, len) < 0) throw SystemError("bind");
}

static void _listen(int sock_fd) {
    if (listen(sock_fd, QUEUE_SIZE) < 0) throw SystemError("listen");
}

static int _accept(int sock_fd) {
    int client_socket = accept(sock_fd, nullptr, nullptr);
    if (client_socket < 0) throw SystemError("accept");
    return client_socket;
}

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
            if (fd.revents & POLLHUP || fd.revents & POLLERR) return;
            if (fd.revents & POLLIN) {
                int client_fd = _accept(fd.fd);
                if (client_fds.size() >= MAX_CLIENTS) {
                    debug("Max clients reached, disconnecting new client");
                    _close(client_fd);
                }
                else {
                    debug(getLocalAddress(client_fd) +
                          " accepted connection from " +
                          getPeerAddress(client_fd));
                    client_fds.insert(client_fd);
                    std::thread(&Server::playerThread, server, client_fd)
                        .detach();
                }
            }
        }
    }
}

void ServerNetworker::stopAccepting() {
    debug("Shutting down server sockets...");
    _shutdown(ipv4_fd, SHUT_RDWR);
    _shutdown(ipv6_fd, SHUT_RDWR);
}

void ServerNetworker::disconnectClients() {
    debug("Disconnecting clients...");
    for (int fd : client_fds)
        _shutdown(fd, SHUT_RDWR);
}

ServerNetworker::~ServerNetworker() {
    debug("Closing server sockets...");
    _close(ipv4_fd);
    _close(ipv6_fd);
    for (int fd : client_fds)
        _close(fd);
}

static bool _valid_address(struct addrinfo* hints, struct addrinfo* res) {
    if (res == nullptr) return false;
    if (hints->ai_family == AF_INET && res->ai_family != AF_INET) return false;
    if (hints->ai_family == AF_INET6 && res->ai_family != AF_INET6)
        return false;
    if (hints->ai_family == AF_UNSPEC && res->ai_family != AF_INET &&
        res->ai_family != AF_INET6)
        return false;

    return res->ai_socktype == hints->ai_socktype &&
           res->ai_protocol == hints->ai_protocol;
}

static std::string _domain_to_string(int domain) {
    switch (domain) {
        case AF_INET:
            return "IPv4";
        case AF_INET6:
            return "IPv6";
        case AF_UNSPEC:
            return "unspecified";
        default:
            return "unknown";
    }
}

ClientNetworker::ClientNetworker(std::string host,
                                 std::string port,
                                 int domain) {
    struct addrinfo hints;
    struct addrinfo *res, *p;

    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = domain;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Get address information
    int status = getaddrinfo(host.c_str(), port.c_str(), &hints, &res);
    if (status != 0)
        throw SystemError("getaddrinfo " + std::string(gai_strerror(status)));

    // Iterate through the linked list of results
    for (p = res; p != nullptr; p = p->ai_next) {
        if (!_valid_address(&hints, p)) continue;
        sock_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sock_fd < 0) continue;
        if (connect(sock_fd, p->ai_addr, p->ai_addrlen) < 0) {
            _close(sock_fd);
            continue;
        }
        break; // If we get here, we have successfully connected
    }

    if (p == nullptr)
        throw SystemError("failed to resolve " + host + ":" + port);

    freeaddrinfo(res);

    debug(getLocalAddress(sock_fd) + " connected to " +
          getPeerAddress(sock_fd) + " (domain " + _domain_to_string(domain) +
          ")");
}

ClientNetworker::~ClientNetworker() {
    debug("Closing client socket...");
    _close(sock_fd);
}

std::string getPeerAddress(int sock_fd) {
    struct sockaddr_storage peerAddr;
    socklen_t addrLen = sizeof(peerAddr);
    char ipStr[INET6_ADDRSTRLEN];
    std::string peerIP, peerPort;

    if (getpeername(sock_fd, (struct sockaddr*)&peerAddr, &addrLen) < 0)
        throw SystemError("getpeername");

    // Determine address family and convert IP address to string
    if (peerAddr.ss_family == AF_INET) {
        struct sockaddr_in* s = (struct sockaddr_in*)&peerAddr;
        inet_ntop(AF_INET, &s->sin_addr, ipStr, sizeof(ipStr));
        peerIP = std::string(ipStr);
        peerPort = std::to_string(ntohs(s->sin_port));
    }
    else if (peerAddr.ss_family == AF_INET6) {
        struct sockaddr_in6* s = (struct sockaddr_in6*)&peerAddr;
        inet_ntop(AF_INET6, &s->sin6_addr, ipStr, sizeof(ipStr));
        peerIP = "[" + std::string(ipStr) + "]";
        peerPort = std::to_string(ntohs(s->sin6_port));
    }
    else throw SystemError("unknown address family");

    return peerIP + ":" + peerPort;
}

std::string getLocalAddress(int sock_fd) {
    struct sockaddr_storage localAddr;
    socklen_t addrLen = sizeof(localAddr);
    char ipStr[INET6_ADDRSTRLEN];
    std::string localIP, localPort;

    if (getsockname(sock_fd, (struct sockaddr*)&localAddr, &addrLen) < 0)
        throw SystemError("getsockname");

    // Determine address family and convert IP address to string
    if (localAddr.ss_family == AF_INET) {
        struct sockaddr_in* s = (struct sockaddr_in*)&localAddr;
        inet_ntop(AF_INET, &s->sin_addr, ipStr, sizeof(ipStr));
        localIP = std::string(ipStr);
        localPort = std::to_string(ntohs(s->sin_port));
    }
    else if (localAddr.ss_family == AF_INET6) {
        struct sockaddr_in6* s = (struct sockaddr_in6*)&localAddr;
        inet_ntop(AF_INET6, &s->sin6_addr, ipStr, sizeof(ipStr));
        localIP = "[" + std::string(ipStr) + "]";
        localPort = std::to_string(ntohs(s->sin6_port));
    }
    else throw SystemError("unknown address family");

    return localIP + ":" + localPort;
}
