#ifndef SERVER_H
#define SERVER_H

#include "network_server.hpp"
#include "protocol.hpp"
#include <condition_variable>

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

    int active_clients;
    std::mutex mtx;
    std::condition_variable active_clients_cv;

    std::atomic<bool> game_over;

    void gameThread();
    void acceptThread();

    void handleGameOver();

public:
    void playerThread(int fd);
};

#endif // SERVER_H
