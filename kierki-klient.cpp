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

        Client client(config);
        client.connectToGame();

        return 0;
    }
    catch (Error& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}
