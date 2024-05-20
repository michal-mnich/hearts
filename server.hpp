#ifndef SERVER_H
#define SERVER_H

#include "network.hpp"
#include <condition_variable>
#include <mutex>
#include <thread>
#include <unordered_set>

#define QUEUE_SIZE 4

class Server {
public:
    Server(int port);
    void start();

private:
    IPv4ServerNetworker ipv4_networker;
    IPv6ServerNetworker ipv6_networker;

    std::unordered_set<int> client_sockets;

    std::thread accept_thread;
    std::thread game_thread;

    int active_clients;
    std::mutex mtx;
    std::condition_variable active_clients_cv;

    std::atomic<bool> game_over;

    void gameThread();
    void acceptThread();
    void clientThread(int client_socket);

    void handleGameOver();
    void handleNewClient(int client_socket);
};

#endif // SERVER_H
