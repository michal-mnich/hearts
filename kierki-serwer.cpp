#include "arg_parser.hpp"
#include "server.hpp"
#include <iostream>

namespace po = boost::program_options;

int main(int argc, char** argv) {
    ServerArgumentParser parser(argc, argv);
    if (!parser.tryParse()) {
        return 1;
    }
    ServerConfig config = parser.getConfig();

    Server server(config.port);
    server.start();

    return 0;
}
