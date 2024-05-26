#ifndef SERVER_H
#define SERVER_H

#include "network_server.hpp"
#include "protocol_server.hpp"
#include <condition_variable>
#include <map>

#define QUEUE_SIZE 4

class Server {
public:
    Server(uint16_t port, unsigned int timeout);
    void start();

private:
    ServerNetworker networker;
    ServerProtocol protocol;

    std::thread accept_thread;
    std::thread game_thread;

    std::atomic<bool> game_over;

    std::mutex mtx;
    std::map<std::string, int> players;

    void gameThread();
    void acceptThread();
    void handleIAM(int fd);

public:
    void playerThread(int fd);
};

#endif // SERVER_H
