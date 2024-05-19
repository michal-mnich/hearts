#include "arg_parser.hpp"
#include <iostream>
#include <arpa/inet.h>
#include "client.hpp"

int main(int argc, char** argv) {
    ClientArgumentParser parser(argc, argv);
    if (!parser.tryParse()) {
        return 1;
    }
    ClientConfig config = parser.getConfig();

    std::cout << "Host: " << config.host << "\n";
    std::cout << "Port: " << config.port << "\n";
    std::cout << "IPv4: " << (config.use_ipv4 ? "Yes" : "No") << "\n";
    std::cout << "IPv6: " << (config.use_ipv6 ? "Yes" : "No") << "\n";
    std::cout << "Seat: " << config.seat << "\n";
    std::cout << "Auto Player: " << (config.auto_player ? "Yes" : "No") << "\n";

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
