#include "arg_parser.hpp"
#include "error.hpp"
#include "server.hpp"
#include <iostream>

int main(int argc, char** argv) {
    try {
        ServerArgumentParser parser(argc, argv);
        parser.parse();
        ServerConfig config = parser.getConfig();

        Server server(config);
        server.start();

        return 0;
    }
    catch (Error& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}
