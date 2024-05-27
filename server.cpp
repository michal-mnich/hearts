#include "server.hpp"
#include "common.hpp"
#include "error.hpp"
#include "network_common.hpp"
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
        deal.cards["N"] = line;

        std::getline(file, line);
        deal.cards["E"] = line;

        std::getline(file, line);
        deal.cards["S"] = line;

        std::getline(file, line);
        deal.cards["W"] = line;

        deals.push_back(deal);
    }
}

Server::Server(ServerConfig& config)
    : networker(config.port), protocol(&networker, config.timeout),
      game_over(false), table(4) {
    parseFile(config.file);
}

void Server::start() {
    int i = 1;
    for (const auto& deal : deals) {
        debug("Starting deal " + std::to_string(i) + "...");

        debug("Starting accept thread...");
        auto accept_thread = std::thread(&Server::acceptThread, this);

        debug("Starting game thread...");
        auto game_thread = std::thread(&Server::gameThread, this, deal);
        game_thread.join();

        debug("Game over!");
        game_over.store(true);

        networker.stopAccepting();
        accept_thread.join();
        debug("Accept thread stopped!");

        networker.disconnectAll();
        networker.joinClients();
        debug("All clients threads stopped!");

        debug("Deal " + std::to_string(i) + " finished!");
        i++;
        break;
    }
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
        waitForRead(fd, -1);
    } while (0);

    networker.removeClient(fd);
}

void Server::gameThread(Deal deal) {
    table.wait();
    debug("All players are ready!");
    for (const auto& [seat, fd] : players) {
        protocol.sendDEAL(fd, deal.type, deal.first, deal.cards[seat]);
    }
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
