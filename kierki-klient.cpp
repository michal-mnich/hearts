#include "arg_parser.hpp"
#include "client.hpp"
#include "error.hpp"
#include <iostream>

int main(int argc, char** argv) {
    try {
        ClientArgumentParser parser(argc, argv);
        parser.parse();
        ClientConfig config = parser.getConfig();

        Client client(config);
        bool correctGame = client.connectToGame();

        return correctGame ? 0 : 1;
    }
    catch (Error& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}
