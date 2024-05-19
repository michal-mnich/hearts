#ifndef SERVER_H
#define SERVER_H

#include <mutex>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <unordered_set>
#include "network.hpp"

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

    std::atomic<int> active_clients;
    std::mutex thread_mutex;

    std::atomic<bool> game_over;

    void gameThread();
    void acceptThread();
    void clientThread(int client_socket);

    void handleGameOver();
    void handleNewClient(int client_socket);
};

#endif // SERVER_H
