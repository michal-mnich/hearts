#include "arg_parser.hpp"
#include "error.hpp"
#include <arpa/inet.h>
#include <filesystem>
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

void ArgumentParser::parse() {
    try {
        _parse();
    }
    catch (const po::error& e) {
        throw SystemError(e.what());
    }
}

ServerArgumentParser::ServerArgumentParser(int argc, char** argv)
    : ArgumentParser(argc, argv), config() {}

ServerConfig ServerArgumentParser::getConfig() {
    return config;
}

void ServerArgumentParser::_parse() {
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

    std::vector<std::string> unrecognized =
        po::collect_unrecognized(parsed.options, po::include_positional);

    if (!unrecognized.empty())
        throw po::error("unrecognized option '" + unrecognized.front() + "'");

    if (!vm.count("file"))
        throw po::error("file parameter (-f <file>) is mandatory");

    if (vm.count("port")) {
        auto port = vm["port"].as<std::vector<int>>().back();
        if (port != 0 && (port < 1024 || port > 65535))
            throw po::error("port must be in the range 1024-65535 or 0");
        config.port = port;
    }
    else config.port = 0;

    auto file = vm["file"].as<std::vector<std::string>>().back();
    if (!std::filesystem::exists(file))
        throw po::error("file '" + file + "' does not exist");
    config.file = file;

    if (vm.count("timeout")) {
        auto timeout = vm["timeout"].as<std::vector<int>>().back();
        if (timeout < 1) throw po::error("timeout must be positive");
        config.timeout = timeout;
    }
    else config.timeout = 5;
}

ClientArgumentParser::ClientArgumentParser(int argc, char** argv)
    : ArgumentParser(argc, argv), config() {}

ClientConfig ClientArgumentParser::getConfig() {
    return config;
}

void ClientArgumentParser::_parse() {
    po::options_description opts;

    // clang-format off
    opts.add_options()
        ("host,h", po::value<std::vector<std::string>>())
        ("port,p", po::value<std::vector<int>>());
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
        if (opt == "-4" || opt == "-6") ipv = opt.substr(1);
        else if (opt == "-N" || opt == "-W" || opt == "-E" || opt == "-S")
            seat = opt.substr(1);
        else if (opt == "-a") a = true;
        else throw po::error("unrecognized option '" + opt + "'");
    }

    if (!vm.count("host"))
        throw po::error("host parameter (-h <port>) is mandatory");

    if (!vm.count("port"))
        throw po::error("port parameter (-p <port>) is mandatory");

    if (seat.empty())
        throw po::error("seat parameter (-N | -E | -S | -W) is mandatory");

    config.host = vm["host"].as<std::vector<std::string>>().back();

    auto port = vm["port"].as<std::vector<int>>().back();
    if (port < 1024 || port > 65535)
        throw po::error("port must be in the range 1024-65535");
    config.port = std::to_string(port);

    if (ipv == "4") config.domain = AF_INET;
    else if (ipv == "6") config.domain = AF_INET6;
    else config.domain = AF_UNSPEC;

    config.seat = seat;
    config.auto_player = a;
}
