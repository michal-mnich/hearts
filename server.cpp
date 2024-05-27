#include "server.hpp"
#include "common.hpp"
#include "error.hpp"
#include "protocol_server.hpp"
#include <fstream>
#include <iostream>
#include <poll.h>
#include <sstream>

void Server::parseFile(const std::string& filename) {
    std::ifstream file(filename);
    std::string line;

    while (std::getline(file, line)) {
        Deal deal;

        deal.type = line[0] - '0';
        deal.first = line.substr(1, 1);

        std::getline(file, line);
        deal.cardsN = line;

        std::getline(file, line);
        deal.cardsE = line;

        std::getline(file, line);
        deal.cardsS = line;

        std::getline(file, line);
        deal.cardsW = line;

        deals.push_back(deal);
    }
}

void printDeal(const Deal& deal) {
    std::cout << "Deal type: " << (int)deal.type << std::endl;
    std::cout << "First player: " << deal.first << std::endl;
    std::cout << "North: " << deal.cardsN << std::endl;
    std::cout << "East: " << deal.cardsE << std::endl;
    std::cout << "South: " << deal.cardsS << std::endl;
    std::cout << "West: " << deal.cardsW << std::endl;
}

Server::Server(ServerConfig& config)
    : networker(config.port), protocol(&networker, config.timeout),
      game_over(false), table(4) {
    parseFile(config.file);
}

void Server::start() {
    debug("Starting accept thread...");
    accept_thread = std::thread(&Server::acceptThread, this);

    debug("Starting game thread...");
    game_thread = std::thread(&Server::gameThread, this);

    game_thread.join();

    debug("Game over!");
    game_over.store(true);

    networker.stopAccepting();
    accept_thread.join();
    debug("Accept thread stopped!");

    networker.disconnectAll();
    networker.joinClients();
    debug("All clients threads stopped!");
}

void Server::acceptThread() {
    networker.startAccepting(this);
    if (!game_over.load())
        throw Error("unexpectedly stopped accepting clients");
}

void Server::playerThread(int fd) {
    std::string seat;

    do {
        try {
            seat = handleIAM(fd);
        }
        catch (Error& e) {
            std::cerr << e.what() << std::endl;
            break;
        }
        debug("Player " + seat + " is ready!");
        table.arrive_and_wait();
    } while (0);

    networker.removeClient(fd);
}

void Server::gameThread() {
    table.wait();
    debug("All players are ready!");
    sleep(5);
}

std::string Server::handleIAM(int fd) {
    std::string seat;
    protocol.recvIAM(fd, seat);

    std::lock_guard<std::mutex> lock(mtx);
    if (players.contains(seat)) {
        protocol.sendBUSY(fd, getKeys(players));
        throw Error("seat " + seat + " already taken");
    }
    else {
        players[seat] = fd;
        debug("Seat " + seat + " taken by client " +
              networker.getClientInfo(fd).second);
    }

    return seat;
}
