#include "arg_parser.hpp"
#include "server.hpp"
#include "error.hpp"
#include <iostream>

namespace po = boost::program_options;

int main(int argc, char** argv) {
    ServerArgumentParser parser(argc, argv);
    if (!parser.tryParse()) {
        return 1;
    }
    ServerConfig config = parser.getConfig();

    debug("Server configuration:");
    debug("Port: " + std::to_string(config.port));
    debug("File: " + config.file);
    debug("Timeout: " + std::to_string(config.timeout));

    Server server(config.port);
    server.start();

    return 0;
}
