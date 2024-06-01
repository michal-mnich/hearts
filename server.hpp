#ifndef SERVER_H
#define SERVER_H

#include "arg_parser.hpp"
#include "game.hpp"
#include "network_server.hpp"
#include "protocol_server.hpp"
#include "common.hpp"
#include <latch>
#include <map>
#include <condition_variable>

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
    std::condition_variable cv_TRICK;
    std::condition_variable cv_allplayers;

    std::map<std::string, int> players;
    int askedTRICK;

    Deal* currentDeal;
    std::vector<Deal> deals;

    void gameThread();
    void acceptThread();
    std::string handleIAM(int fd);
    void handleTRICK(int fd);
    void parseFile(const std::string& file);

public:
    void playerThread(int fd);
};

#endif // SERVER_H
