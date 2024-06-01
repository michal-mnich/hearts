#ifndef SERVER_H
#define SERVER_H

#include "arg_parser.hpp"
#include "game.hpp"
#include "network_server.hpp"
#include "protocol_server.hpp"
#include "common.hpp"
#include <latch>
#include <map>

#define QUEUE_SIZE 4

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
    SimpleCV cv;

    Deal* currentDeal;
    std::vector<Deal> deals;

    void gameThread();
    void acceptThread();
    std::string handleIAM(int fd);
    void handleTRICK(int fd, std::string& seat);
    void parseFile(const std::string& file);

public:
    void playerThread(int fd);
};

#endif // SERVER_H
