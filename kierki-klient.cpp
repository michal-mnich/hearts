#include "arg_parser.hpp"
#include "client.hpp"
#include "error.hpp"
#include "network_common.hpp"
#include <iostream>

int main(int argc, char** argv) {
    try {
        ClientArgumentParser parser(argc, argv);
        parser.parse();
        ClientConfig config = parser.getConfig();

        debug("Host: " + config.host);
        debug("Port: " + config.port);
        debug("IP Version: " + _domainToString(config.domain));
        debug("Seat: " + config.seat);
        debug("Auto Player: " + std::string(config.auto_player ? "Yes" : "No"));

        Client client(config);
        client.connectToGame();

        return 0;
    }
    catch (Error& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}
