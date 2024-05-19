#include "arg_parser.hpp"
#include <iostream>

namespace po = boost::program_options;

int main(int argc, char** argv) {
    ServerArgumentParser parser(argc, argv);
    if (!parser.tryParse()) {
        return 1;
    }
    ServerConfig config = parser.getConfig();

    std::cout << "Port: " << config.port << '\n';
    std::cout << "File: " << config.file << '\n';
    std::cout << "Timeout: " << config.timeout << '\n';

    return 0;
}
