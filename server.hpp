#ifndef SERVER_H
#define SERVER_H

#include "arg_parser.hpp"
#include "common.hpp"
#include "game.hpp"
#include "network_server.hpp"
#include "protocol_server.hpp"
#include <condition_variable>
#include <map>

class Server {
public:
    explicit Server(ServerConfig& config);
    void start();

private:
    ServerNetworker networker;
    ServerProtocol protocol;

    std::atomic<bool> game_over;

    std::mutex mtxRunning;
    std::map<std::string, int> player_fds;
    std::condition_variable cvRunning;
    int askedTRICK = -1;
    std::condition_variable cvTrick;

    std::mutex mtxSuspended;
    bool isSuspended = false;
    std::condition_variable cvSuspended;

    Deal* currentDeal = nullptr;
    std::vector<Deal> deals;
    std::map<std::string, unsigned int> totalScores;

    void gameThread();
    void acceptThread();
    std::string handleIAM(int fd);
    void handleTRICK(int fd);
    void parseFile(const std::string& file);

public:
    void playerThread(int fd);
};

#endif // SERVER_H
