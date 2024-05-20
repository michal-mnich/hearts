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
    try {
        ClientArgumentParser parser(argc, argv);
        parser.parse();
        ClientConfig config = parser.getConfig();

        debug("Host: " + config.host);
        debug("Port: " + std::to_string(config.port));
        debug("IP Version: " + domainToString(config.domain));
        debug("Seat: " + config.seat);
        debug("Auto Player: " + std::string(config.auto_player ? "Yes" : "No"));

        Client client(config.host, config.port, config.domain);
        client.connectToGame();

        return 0;
    }
    catch (const SystemError& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}
