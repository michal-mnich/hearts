#ifndef SERVER_H
#define SERVER_H

#include "arg_parser.hpp"
#include "game.hpp"
#include "network_server.hpp"
#include "protocol_server.hpp"
#include "common.hpp"
#include <map>
#include <condition_variable>

class Server {
public:
    explicit Server(ServerConfig& config);
    void start();

private:
    ServerNetworker networker;
    ServerProtocol protocol;

    std::atomic<bool> game_over;

    std::mutex mtx;
    std::condition_variable cv_TRICK;
    std::condition_variable cv_allplayers;

    std::map<std::string, int> player_fds;
    int askedTRICK = -1;

    Deal* currentDeal = nullptr;
    std::vector<Deal> deals;
    std::map<std::string, unsigned int> scores;

    void gameThread();
    void acceptThread();
    std::string handleIAM(int fd);
    void handleTRICK(int fd);
    void parseFile(const std::string& file);

public:
    void playerThread(int fd);
};

#endif // SERVER_H
