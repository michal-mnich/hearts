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

    bool dealStarted = false;
    bool dealSuspended = false;
    std::atomic<bool> gameOver;

    std::map<std::string, int> player_fds;
    std::map<std::string, bool> trickDone;
    int fdExpectedTrick = -1;

    std::mutex mtx;
    std::condition_variable cvReady;
    std::condition_variable cvTrick;
    std::condition_variable cvSuspended;

    Deal* currentDeal = nullptr;
    std::vector<Deal> deals;
    std::map<std::string, unsigned int> totalScores;

    void dealThread();
    void acceptThread();
    std::string handleIAM(int fd);
    void handleTRICK(int fd);
    void parseFile(const std::string& file);

public:
    void playerThread(int fd);
};

#endif // SERVER_H
