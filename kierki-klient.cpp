#include "arg_parser.hpp"
#include "client.hpp"
#include "error.hpp"
#include <iostream>

std::string domainToString(int domain) {
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

int main(int argc, char** argv) {
    ClientArgumentParser parser(argc, argv);
    if (!parser.tryParse()) return 1;

    ClientConfig config = parser.getConfig();

    debug("Host: " + config.host);
    debug("Port: " + std::to_string(config.port));
    debug("IP Version: " + domainToString(config.domain));
    debug("Seat: " + config.seat);
    debug("Auto Player: " + std::string(config.auto_player ? "Yes" : "No"));

    Client client(config.host, config.port, config.domain);
    client.connectToGame();

    // The rest of the client code goes here...

    return 0;
}
