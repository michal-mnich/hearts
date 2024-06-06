#include "network_common.hpp"
#include "error.hpp"
#include <chrono>
#include <fcntl.h>
#include <netdb.h>
#include <poll.h>
#include <unistd.h>

#define BUF_SIZE     1
#define MAX_MSG_SIZE 64

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

static void _getsockname(int sock_fd, struct sockaddr_storage* addr) {
    socklen_t addrLen = sizeof(*addr);
    if (getsockname(sock_fd, (struct sockaddr*)addr, &addrLen) < 0)
        throw Error("getsockname");
}

static void _getpeername(int sock_fd, struct sockaddr_storage* addr) {
    socklen_t addrLen = sizeof(*addr);
    if (getpeername(sock_fd, (struct sockaddr*)addr, &addrLen) < 0)
        throw Error("getpeername");
}

bool _validAddress(const struct addrinfo* hints, struct addrinfo* res) {
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

    // Check if socket type and protocol match
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

static std::string _getAddressString(struct sockaddr_storage* addr) {
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

// Write n bytes to a descriptor
static void writen(int fd, const void* vptr, size_t n) {
    ssize_t nleft;
    const char* ptr;

    ptr = static_cast<const char*>(vptr);
    nleft = n;
    while (nleft > 0) {
        auto nwritten = write(fd, ptr, nleft);
        if (nwritten < 0 && errno == EINTR) {
            continue;
        }
        else if (nwritten < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            throw Error("write (would block)");
        }
        else if (nwritten < 0 && errno == EPIPE) {
            throw Error("write (connection closed by peer)");
        }
        else if (nwritten < 0) {
            throw Error("write");
        }
        nleft -= nwritten;
        ptr += nwritten;
    }
}

void sendMessage(int fd, const std::string& message) {
    writen(fd, message.c_str(), message.size());
}

void waitPollIn(int fd, int timeout) {
    struct pollfd pollfd;
    pollfd.fd = fd;
    pollfd.events = POLLIN;
    int ret = poll(&pollfd, 1, timeout);
    if (ret < 0) {
        throw Error("poll");
    }
    if (ret == 0) {
        throw Error("poll (timeout)");
    }
    if (!(pollfd.revents & POLLIN)) {
        throw Error("poll (revents)");
    }
}

void waitPollOut(int fd) {
    struct pollfd pollfd;
    pollfd.fd = fd;
    pollfd.events = POLLOUT;
    int ret = poll(&pollfd, 1, -1);
    if (ret < 0) {
        throw Error("poll");
    }
    if (!(pollfd.revents & POLLOUT)) {
        throw Error("poll (revents)");
    }
}

static std::string _read(int fd) {
    char buffer[BUF_SIZE];
    int nread;
begin:
    nread = read(fd, buffer, BUF_SIZE);
    if (nread < 0 && errno == EINTR) {
        goto begin;
    }
    if (nread < 0) {
        throw Error("read");
    }
    if (nread == 0) {
        throw Error("read (connection closed by peer)");
    }
    std::string res(buffer, nread);
    return res;
}

std::string recvMessage(int fd, int timeout) {
    std::chrono::system_clock::time_point start, end;
    std::string message;
    bool timeout_mode = (timeout != -1);
    if (timeout_mode) timeout *= 1000;
    while (true) {
        if (timeout_mode) start = std::chrono::system_clock::now();

        waitPollIn(fd, timeout);

        if (timeout_mode) {
            end = std::chrono::system_clock::now();
            auto dur = duration_cast<std::chrono::milliseconds>(end - start);
            timeout -= dur.count();
            if (timeout <= 0) throw Error("recvMessage (timeout)");
        }

        auto chunk = _read(fd);
        message += chunk;

        auto len = message.size();
        if (len >= 2 && message.substr(len - 2) == "\r\n") {
            break;
        }
        else if (message.size() > MAX_MSG_SIZE) {
            message += "\r\n";
            break;
        }
    }
    return message;
}

void setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) throw Error("fcntl (setNonBlocking)");
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)
        throw Error("fcntl (F_SETFL)");
}

void unsetNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) throw Error("fcntl (unsetNonBlocking)");
    if (fcntl(fd, F_SETFL, flags & ~O_NONBLOCK) < 0)
        throw Error("fcntl (F_SETFL)");
}
