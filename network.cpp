#include "network.hpp"
#include "error.hpp"
#include <cstring>

#define QUEUE_SIZE 4

Networker::Networker(uint16_t port) : sock_fd(-1), port(port) {}

int Networker::getSocket() {
    return sock_fd;
}

ServerNetworker::ServerNetworker(uint16_t port) : Networker(port) {
    std::memset(&addr, 0, sizeof(addr));
    addr.sin6_family = AF_INET6;
    addr.sin6_addr = in6addr_any;
    addr.sin6_port = htons(port);
}

void ServerNetworker::createSocket() {
    int opt, optname;

    sock_fd = socket(AF_INET6, SOCK_STREAM, 0);
    if (sock_fd < 0) throw SystemError("socket");

    opt = 1;
    optname = SO_REUSEADDR;
    if (setsockopt(sock_fd, SOL_SOCKET, optname, &opt, sizeof(opt)) < 0)
        throw SystemError("setsockopt SO_REUSEADDR");

    opt = 0;
    optname = IPV6_V6ONLY;
    if (setsockopt(sock_fd, IPPROTO_IPV6, optname, &opt, sizeof(opt)) < 0)
        throw SystemError("setsockopt IPV6_V6ONLY");
}

void ServerNetworker::bindSocket() {
    if (bind(sock_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
        throw SystemError("bind");
}

void ServerNetworker::listenSocket() {
    if (listen(sock_fd, QUEUE_SIZE) < 0) throw SystemError("listen");
}

ClientNetworker::ClientNetworker(std::string host, uint16_t port, int domain)
    : Networker(port), host(host), domain(domain) {}

static bool validAddress(struct addrinfo* hints, struct addrinfo* res) {
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

void ClientNetworker::connectToServer() {
    struct addrinfo hints;
    struct addrinfo *res, *p;
    std::string port_str = std::to_string(port);

    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = domain;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Get address information
    int status = getaddrinfo(host.c_str(), port_str.c_str(), &hints, &res);
    if (status != 0)
        throw SystemError("getaddrinfo " + std::string(gai_strerror(status)));

    // Iterate through the linked list of results
    for (p = res; p != nullptr; p = p->ai_next) {
        if (!validAddress(&hints, p)) continue;
        sock_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sock_fd < 0) continue;
        if (connect(sock_fd, p->ai_addr, p->ai_addrlen) < 0) {
            closeSocket(sock_fd);
            continue;
        }
        break; // If we get here, we have successfully connected
    }

    if (p == nullptr) throw SystemError("failed to resolve hostname");

    freeaddrinfo(res);
}

std::string getPeerAddress(int sock_fd) {
    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
    struct sockaddr_in6 peerAddr;
    socklen_t addrLen = sizeof(peerAddr);

    if (getpeername(sock_fd, (struct sockaddr*)&peerAddr, &addrLen) < 0)
        throw SystemError("getpeername");

    if (getnameinfo((struct sockaddr*)&peerAddr,
                    addrLen,
                    hbuf,
                    sizeof(hbuf),
                    sbuf,
                    sizeof(sbuf),
                    NI_NUMERICHOST | NI_NUMERICSERV) < 0)
        throw SystemError("getnameinfo");

    return std::string(hbuf) + ":" + std::string(sbuf);
}

std::string getLocalAddress(int sock_fd) {
    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
    struct sockaddr_in6 local_addr;
    socklen_t addr_len = sizeof(local_addr);

    if (getsockname(sock_fd, (struct sockaddr*)&local_addr, &addr_len) < 0)
        throw SystemError("getsockname");

    if (getnameinfo((struct sockaddr*)&local_addr,
                    addr_len,
                    hbuf,
                    sizeof(hbuf),
                    sbuf,
                    sizeof(sbuf),
                    NI_NUMERICHOST | NI_NUMERICSERV) < 0)
        throw SystemError("getnameinfo");

    return std::string(hbuf) + ":" + std::string(sbuf);
}

void closeSocket(int sock_fd) {
    if (sock_fd >= 0) {
        if (close(sock_fd) < 0) throw SystemError("close");
    }
}

void shutdownSocket(int sock_fd) {
    if (sock_fd >= 0) {
        if (shutdown(sock_fd, SHUT_RDWR) < 0) throw SystemError("shutdown");
    }
}
