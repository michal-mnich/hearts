#include "network_client.hpp"
#include "error.hpp"
#include "network_common.hpp"
#include <cstring>
#include <netdb.h>

ClientNetworker::ClientNetworker(const std::string& name,
                                 const std::string& service,
                                 int domain) {
    struct addrinfo hints;
    struct addrinfo *res, *p;

    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = domain;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Get address information
    int status = getaddrinfo(name.c_str(), service.c_str(), &hints, &res);
    if (status != 0)
        throw Error("getaddrinfo " + std::string(gai_strerror(status)));

    // Iterate through the linked list of results
    for (p = res; p != nullptr; p = p->ai_next) {
        if (!_validAddress(&hints, p)) continue;
        sock_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sock_fd < 0) continue;
        if (connect(sock_fd, p->ai_addr, p->ai_addrlen) < 0) {
            _close(sock_fd);
            continue;
        }
        break; // If we get here, we have successfully connected
    }

    freeaddrinfo(res);

    if (p == nullptr) throw Error("failed to resolve " + name + ":" + service);

    localAddress = getLocalAddress(sock_fd);
    peerAddress = getPeerAddress(sock_fd);
    debug(localAddress + " connected to " + peerAddress + " (domain " +
          _domainToString(domain) + ")");
}

ClientNetworker::~ClientNetworker() {
    debug("Closing client socket...");
    _close(sock_fd);
}
