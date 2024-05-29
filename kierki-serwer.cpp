#include "arg_parser.hpp"
#include "error.hpp"
#include "server.hpp"
#include <iostream>

int main(int argc, char** argv) {
    try {
        ServerArgumentParser parser(argc, argv);
        parser.parse();
        ServerConfig config = parser.getConfig();

        debug("Port: " + std::to_string(config.port));
        debug("File: " + config.file);
        debug("Timeout: " + std::to_string(config.timeout));

        Server server(config);
        server.start();

        return 0;
    }
    catch (Error& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}
