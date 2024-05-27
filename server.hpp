#ifndef SERVER_H
#define SERVER_H

#include "arg_parser.hpp"
#include "network_server.hpp"
#include "protocol_server.hpp"
#include <condition_variable>
#include <latch>
#include <map>

#define QUEUE_SIZE 4

struct Deal {
    uint8_t type;
    std::string first;
    std::map<std::string, std::string> cards; // seat, cards
};

class Server {
public:
    Server(ServerConfig& config);
    void start();

private:
    ServerNetworker networker;
    ServerProtocol protocol;

    std::atomic<bool> game_over;

    std::mutex mtx;
    std::map<std::string, int> players;

    std::latch table;

    std::vector<Deal> deals;

    void gameThread(Deal deal);
    void acceptThread();
    std::string handleIAM(int fd);
    void parseFile(const std::string& file);

public:
    void playerThread(int fd);
};

#endif // SERVER_H
