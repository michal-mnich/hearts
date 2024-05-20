#include "arg_parser.hpp"
#include "error.hpp"
#include <iostream>
#include <arpa/inet.h>
#include "client.hpp"

int main(int argc, char** argv) {
    ClientArgumentParser parser(argc, argv);
    if (!parser.tryParse()) {
        return 1;
    }
    ClientConfig config = parser.getConfig();

    debug("Client configuration:");
    debug("Host: " + config.host);
    debug("Port: " + std::to_string(config.port));
    debug("IPv4: " + std::string(config.use_ipv4 ? "Yes" : "No"));
    debug("IPv6: " + std::string(config.use_ipv6 ? "Yes" : "No"));
    debug("Seat: " + std::string(1, config.seat));
    debug("Auto Player: " + std::string(config.auto_player ? "Yes" : "No"));

    int domain;
    if (config.use_ipv4) {
        domain = AF_INET;
    }
    else if (config.use_ipv6) {
        domain = AF_INET6;
    }
    else {
        domain = AF_UNSPEC;
    }

    Client client(config.host, config.port, domain);
    client.connectToGame();

    // The rest of the client code goes here...

    return 0;
}
