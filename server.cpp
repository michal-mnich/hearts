#include "server.hpp"
#include "common.hpp"
#include "error.hpp"
#include "network_common.hpp"
#include "protocol_server.hpp"
#include <fstream>
#include <iostream>

void Server::parseFile(const std::string& filename) {
    std::ifstream file(filename);
    std::string line;

    while (std::getline(file, line)) {
        Deal deal;

        deal.type = line[0] - '0';
        deal.firstPlayer = line.substr(1, 1);
        deal.currentPlayer = deal.firstPlayer;

        std::getline(file, line);
        deal.hand["N"] = line;

        std::getline(file, line);
        deal.hand["E"] = line;

        std::getline(file, line);
        deal.hand["S"] = line;

        std::getline(file, line);
        deal.hand["W"] = line;

        deals.push_back(deal);
    }
}

Server::Server(ServerConfig& config)
    : networker(config.port), protocol(&networker, config.timeout),
      game_over(false), table(4), next(0) {
    parseFile(config.file);
}

void Server::start() {
    for (unsigned int i = 0; i < deals.size(); i++) {
        debug("Starting deal " + std::to_string(i + 1) + "...");
        currentDeal = &deals[i];

        debug("Starting accept thread...");
        auto accept_thread = std::thread(&Server::acceptThread, this);

        debug("Starting game thread...");
        auto game_thread = std::thread(&Server::gameThread, this, currentDeal);
        game_thread.join();

        debug("Game over!");
        game_over.store(true);

        networker.stopAccepting();
        accept_thread.join();
        debug("Accept thread stopped!");

        networker.disconnectAll();
        networker.joinClients();
        debug("All clients threads stopped!");

        debug("Deal " + std::to_string(i + 1) + " finished!");
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

    try {
        seat = handleIAM(fd);
    }
    catch (Error& e) {
        std::cerr << e.what() << std::endl;
        goto disconnect;
    }

    debug("Player " + seat + " is ready!");
    table.arrive_and_wait();

    while (true) {
        try {
            handleTRICK(fd);
        }
        catch (Error& e) {
            if (seat == currentDeal->currentPlayer) {
                std::cerr << e.what() << std::endl;
                goto disconnect;
            }
        }
    }

disconnect:
    networker.removeClient(fd);
}

void Server::gameThread(Deal* deal) {
    table.wait();
    debug("All players are ready!");
    for (const auto& [seat, fd] : players) {
        protocol.sendDEAL(fd, deal->type, deal->firstPlayer, deal->hand[seat]);
    }

    while (deal->currentTrick < 13) {
        if (deal->currentPlayer == deal->firstPlayer) {
            deal->currentTrick++;
        }
        protocol.sendTRICK(players[deal->currentPlayer],
                           deal->currentTrick,
                           deal->cardsOnTable);
        next.acquire();
        deal->getNextPlayer(deal->currentPlayer);
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

void Server::handleTRICK(int fd) {
    uint8_t trick;
    std::string cardPlaced;
    protocol.recvTRICK(fd, &trick, cardPlaced);
    currentDeal->cardsOnTable += cardPlaced;
    next.release();
}
