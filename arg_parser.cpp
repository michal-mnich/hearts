#include "arg_parser.hpp"
#include <arpa/inet.h>
#include <iostream>

ArgumentParser::ArgumentParser(int argc, char** argv)
    : argc(argc), argv(argv) {}

auto ArgumentParser::getParser(po::options_description& opts) {
    return po::command_line_parser(argc, argv)
        .options(opts)
        .style(po::command_line_style::allow_short ^
               po::command_line_style::allow_dash_for_short ^
               po::command_line_style::short_allow_next);
}

bool ArgumentParser::tryParse() {
    try {
        parse();
        return true;
    }
    catch (const po::error& ex) {
        std::cerr << "ERROR: " << ex.what() << "\n";
        return false;
    }
    catch (const std::exception& ex) {
        std::cerr << "EXCEPTION: " << ex.what() << "\n";
        return false;
    }
    catch (...) {
        std::cerr << "ERROR: Unknown\n";
        return false;
    }
}

ServerArgumentParser::ServerArgumentParser(int argc, char** argv)
    : ArgumentParser(argc, argv), config() {}

ServerConfig ServerArgumentParser::getConfig() {
    return config;
}

void ServerArgumentParser::parse() {
    po::options_description opts;

    // clang-format off
    opts.add_options()
        ("port,p", po::value<std::vector<int>>())
        ("file,f", po::value<std::vector<std::string>>())
        ("timeout,t", po::value<std::vector<int>>());
    // clang-format on

    po::variables_map vm;
    auto parsed = getParser(opts).run();
    po::store(parsed, vm);
    po::notify(vm);

    if (!vm.count("file"))
        throw po::error("File parameter (-f <file>) is mandatory.");

    if (vm.count("port"))
        config.port = vm["port"].as<std::vector<int>>().back();
    else config.port = 0;

    config.file = vm["file"].as<std::vector<std::string>>().back();

    if (vm.count("timeout"))
        config.timeout = vm["timeout"].as<std::vector<int>>().back();
    else config.timeout = 5;
}

ClientArgumentParser::ClientArgumentParser(int argc, char** argv)
    : ArgumentParser(argc, argv), config() {}

ClientConfig ClientArgumentParser::getConfig() {
    return config;
}

void ClientArgumentParser::parse() {
    po::options_description opts;

    // clang-format off
    opts.add_options()
        ("host,h", po::value<std::vector<std::string>>())
        ("port,p", po::value<std::vector<uint16_t>>());
    // clang-format on

    po::variables_map vm;
    auto parsed = getParser(opts).allow_unregistered().run();
    po::store(parsed, vm);
    po::notify(vm);

    // Process other options to capture the last occurrence
    std::vector<std::string> unrecognized =
        po::collect_unrecognized(parsed.options, po::include_positional);
    std::string ipv;
    std::string seat;
    bool a = false;
    for (const auto& opt : unrecognized) {
        if (opt == "-4" || opt == "-6") {
            ipv = opt.substr(1);
        }
        else if (opt == "-N" || opt == "-W" || opt == "-E" || opt == "-S") {
            seat = opt.substr(1);
        }
        else if (opt == "-a") {
            a = true;
        }
        else {
            throw po::error("Unrecognized option: " + opt);
        }
    }

    if (!vm.count("host")) {
        throw po::error("Host parameter (-h <port>) is mandatory.");
    }

    if (!vm.count("port")) {
        throw po::error("Port parameter (-p <port>) is mandatory.");
    }

    if (seat.empty()) {
        throw po::error("Seat parameter (-N | -E | -S | -W) is mandatory.");
    }

    config.host = vm["host"].as<std::vector<std::string>>().back();
    config.port = vm["port"].as<std::vector<uint16_t>>().back();

    if (ipv == "4") config.domain = AF_INET;
    else if (ipv == "6") config.domain = AF_INET6;
    else config.domain = AF_UNSPEC;

    config.seat = seat;
    config.auto_player = a;
}
