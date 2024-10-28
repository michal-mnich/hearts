#ifndef ARG_PARSER_H
#define ARG_PARSER_H

#include <boost/program_options.hpp>

namespace po = boost::program_options;

struct ServerConfig {
    uint16_t port;
    std::string file;
    uint32_t timeout;
};

struct ClientConfig {
    std::string host;
    std::string port;
    int domain;
    std::string seat;
    bool auto_player;
};

class ArgumentParser {
protected:
    int argc;
    char** argv;
    virtual void _parse() = 0;
    auto getParser(po::options_description& opts);

public:
    ArgumentParser(int argc, char** argv);
    void parse();
};

class ServerArgumentParser : public ArgumentParser {
private:
    ServerConfig config;
    void _parse() override;

public:
    ServerArgumentParser(int argc, char** argv);
    ServerConfig getConfig();
};

class ClientArgumentParser : public ArgumentParser {
private:
    ClientConfig config;
    void _parse() override;

public:
    ClientArgumentParser(int argc, char** argv);
    ClientConfig getConfig();
};

#endif // ARG_PARSER_H
