#include "network_common.hpp"
#include "error.hpp"
#include <netdb.h>
#include <unistd.h>

/* System functions wrappers with error handling */

void _close(int sock_fd) {
    if (close(sock_fd) < 0) throw Error("close");
}

void _shutdown(int sock_fd, int how) {
    if (shutdown(sock_fd, how) < 0) throw Error("shutdown");
}

int _socket(int domain) {
    int opt, optname;

    int sock_fd = socket(domain, SOCK_STREAM, 0);
    if (sock_fd < 0) throw Error("socket");

    // Allow reuse of local addresses (no TIME_WAIT)
    opt = 1;
    optname = SO_REUSEADDR;
    if (setsockopt(sock_fd, SOL_SOCKET, optname, &opt, sizeof(opt)) < 0)
        throw Error("setsockopt (SO_REUSEADDR)");

    // Allow only IPv6 connections on an IPv6 socket
    if (domain == AF_INET6) {
        opt = 1;
        optname = IPV6_V6ONLY;
        if (setsockopt(sock_fd, IPPROTO_IPV6, optname, &opt, sizeof(opt)) < 0)
            throw Error("setsockopt (IPV6_V6ONLY)");
    }

    return sock_fd;
}

void _bind(int sock_fd, struct sockaddr* addr, socklen_t len) {
    if (bind(sock_fd, addr, len) < 0) throw Error("bind");
}

void _listen(int sock_fd) {
    if (listen(sock_fd, QUEUE_SIZE) < 0) throw Error("listen");
}

int _accept(int sock_fd) {
    int client_socket = accept(sock_fd, nullptr, nullptr);
    if (client_socket < 0) throw Error("accept");
    return client_socket;
}

void _getsockname(int sock_fd, struct sockaddr_storage* addr) {
    socklen_t addrLen = sizeof(*addr);
    if (getsockname(sock_fd, (struct sockaddr*)addr, &addrLen) < 0)
        throw Error("getsockname");
}

void _getpeername(int sock_fd, struct sockaddr_storage* addr) {
    socklen_t addrLen = sizeof(*addr);
    if (getpeername(sock_fd, (struct sockaddr*)addr, &addrLen) < 0)
        throw Error("getpeername");
}

bool _validAddress(struct addrinfo* hints, struct addrinfo* res) {
    // Check if hints and result are valid
    if (!hints || !res) return false;

    // Wanted IPv4, got something else
    if (hints->ai_family == AF_INET && res->ai_family != AF_INET) {
        return false;
    }

    // Wanted IPv6, got something else
    if (hints->ai_family == AF_INET6 && res->ai_family != AF_INET6) {
        return false;
    }

    // Wanted IPv4 or IPv6, got neither IPv4 nor IPv6
    if (hints->ai_family == AF_UNSPEC && res->ai_family != AF_INET &&
        res->ai_family != AF_INET6)
    {
        return false;
    }

    // Check if socket type and protocol matches
    return res->ai_socktype == hints->ai_socktype &&
           res->ai_protocol == hints->ai_protocol;
}

std::string _domainToString(int domain) {
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

std::string _getAddressString(struct sockaddr_storage* addr) {
    char ipStr[INET6_ADDRSTRLEN];
    std::string ip, port;

    // Determine address family and convert IP address to string
    if (addr->ss_family == AF_INET) {
        struct sockaddr_in* s = (struct sockaddr_in*)addr;
        inet_ntop(AF_INET, &s->sin_addr, ipStr, sizeof(ipStr));
        ip = std::string(ipStr);
        port = std::to_string(ntohs(s->sin_port));
    }
    else if (addr->ss_family == AF_INET6) {
        struct sockaddr_in6* s = (struct sockaddr_in6*)addr;
        inet_ntop(AF_INET6, &s->sin6_addr, ipStr, sizeof(ipStr));
        ip = "[" + std::string(ipStr) + "]";
        port = std::to_string(ntohs(s->sin6_port));
    }
    else {
        throw Error("unknown address family (" +
                    std::to_string(addr->ss_family) + ")");
    }

    return ip + ":" + port;
}

std::string getPeerAddress(int sock_fd) {
    struct sockaddr_storage peerAddr;
    _getpeername(sock_fd, &peerAddr);
    return _getAddressString(&peerAddr);
}

std::string getLocalAddress(int sock_fd) {
    struct sockaddr_storage localAddr;
    _getsockname(sock_fd, &localAddr);
    return _getAddressString(&localAddr);
}

// Read n bytes from a descriptor
void readn(int fd, void* vptr, size_t n) {
    ssize_t nleft, nread;
    char* ptr;

    ptr = static_cast<char*>(vptr);
    nleft = n;

    while (nleft > 0) {
        nread = read(fd, ptr, nleft);
        if (nread < 0 && errno == EINTR) {
            // interrupted by signal
            continue;
        }
        if (nread < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            errno = 0;
            throw Error("read (timeout)");
        }
        else if (nread < 0) {
            throw Error("read");
        }
        else if (nread == 0) {
            throw Error("read (connection closed by peer)");
        }
        nleft -= nread;
        ptr += nread;
    }
}

// Write n bytes to a descriptor
void writen(int fd, const void* vptr, size_t n) {
    ssize_t nleft, nwritten;
    const char* ptr;

    ptr = static_cast<const char*>(vptr);
    nleft = n;
    while (nleft > 0) {
        nwritten = write(fd, ptr, nleft);
        if (nwritten < 0 && errno == EINTR) {
            continue;
        }
        else if (nwritten < 0) {
            throw Error("write");
        }
        nleft -= nwritten;
        ptr += nwritten;
    }
}

void socket_set_timeout(int socket_fd, unsigned int timeout) {
    struct timeval to = {.tv_sec = timeout, .tv_usec = 0};
    if (setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &to, sizeof to) < 0) {
        throw Error("setsockopt (SO_RCVTIMEO)");
    }
}

void socket_clear_timeout(int socket_fd) {
    struct timeval to = {.tv_sec = 0, .tv_usec = 0};
    if (setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &to, sizeof to) < 0) {
        throw Error("setsockopt (SO_RCVTIMEO)");
    }
}

std::string readUntilEnd(int fd) {
    char buf;
    std::string message;
    while (true) {
        readn(fd, &buf, 1);
        message += buf;
        if (buf == '\n') break;
    }
    return message;
}
