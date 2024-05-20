#include "network.hpp"
#include "error.hpp"

#define QUEUE_SIZE 4

ServerNetworker::ServerNetworker(uint16_t port) : Networker(port) {}

int ServerNetworker::_createSocket(int domain) {
    sock_fd = socket(domain, SOCK_STREAM, 0);
    if (sock_fd < 0) throw SystemError("socket");
    int opt = 1;
    int optname = SO_REUSEADDR | SO_REUSEPORT;
    if (setsockopt(sock_fd, SOL_SOCKET, optname, &opt, sizeof(opt)) < 0)
        throw SystemError("setsockopt");
    return sock_fd;
}

void ServerNetworker::_bindSocket(int sock_fd,
                                  struct sockaddr* addr,
                                  socklen_t addr_size) {
    if (bind(sock_fd, addr, addr_size) < 0) throw SystemError("bind");
}

void ServerNetworker::listenSocket() {
    if (listen(sock_fd, QUEUE_SIZE) < 0) throw SystemError("listen");
}

IPv4ServerNetworker::IPv4ServerNetworker(uint16_t port)
    : ServerNetworker(port) {
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
}

void IPv4ServerNetworker::createSocket() {
    sock_fd = _createSocket(AF_INET);
}

void IPv4ServerNetworker::bindSocket() {
    _bindSocket(sock_fd, (struct sockaddr*)&addr, sizeof(addr));
}

IPv6ServerNetworker::IPv6ServerNetworker(uint16_t port)
    : ServerNetworker(port) {
    std::memset(&addr, 0, sizeof(addr));
    addr.sin6_family = AF_INET6;
    addr.sin6_addr = in6addr_any;
    addr.sin6_port = htons(port);
}

void IPv6ServerNetworker::createSocket() {
    sock_fd = _createSocket(AF_INET6);
}

void IPv6ServerNetworker::bindSocket() {
    _bindSocket(sock_fd, (struct sockaddr*)&addr, sizeof(addr));
}

ClientNetworker::ClientNetworker(std::string host, uint16_t port, int domain)
    : Networker(port), host(host), domain(domain) {}

static bool validAddress(struct addrinfo* hints, struct addrinfo* res) {
    return res != nullptr && res->ai_family == hints->ai_family &&
           res->ai_socktype == hints->ai_socktype &&
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
    struct sockaddr_storage peerAddr;
    socklen_t addrLen = sizeof(peerAddr);
    char ipStr[INET6_ADDRSTRLEN];
    std::string peerIP, peerPort;

    if (getpeername(sock_fd, (struct sockaddr*)&peerAddr, &addrLen) < 0)
        throw new SystemError("getpeername");

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
    else throw new SystemError("unknown address family");

    return peerIP + ":" + peerPort;
}

std::string getLocalAddress(int sock_fd) {
    struct sockaddr_storage localAddr;
    socklen_t addrLen = sizeof(localAddr);
    char ipStr[INET6_ADDRSTRLEN];
    std::string localIP, localPort;

    if (getsockname(sock_fd, (struct sockaddr*)&localAddr, &addrLen) < 0)
        throw new SystemError("getsockname");

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
    else throw new SystemError("unknown address family");

    return localIP + ":" + localPort;
}

Networker::Networker(uint16_t port) : sock_fd(-1), port(port) {}

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

int Networker::getSocket() {
    return sock_fd;
}
