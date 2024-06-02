#include "server.hpp"
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
        deal.currentTrick = 1;
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
      game_over(false) {
    parseFile(config.file);
}

void Server::start() {
    for (unsigned int i = 0; i < deals.size(); i++) {
        debug("Starting deal " + std::to_string(i + 1) + "...");
        currentDeal = &deals[i];

        debug("Starting accept thread...");
        auto accept_thread = std::thread(&Server::acceptThread, this);

        debug("Starting game thread...");
        auto game_thread = std::thread(&Server::gameThread, this);
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

    while (true) {
        try {
            handleTRICK(fd);
        }
        catch (Error& e) {
            std::cerr << e.what() << std::endl;
            goto disconnect;
        }
    }

disconnect:
    networker.disconnectOne(fd);
}

void Server::gameThread() {
    std::unique_lock<std::mutex> lock(mtx);
    cv_allplayers.wait(lock, [this] { return player_fds.size() == 4; });

    debug("All players are ready!");

    for (const auto& [seat, fd] : player_fds) {
        protocol.sendDEAL(fd,
                          currentDeal->type,
                          currentDeal->firstPlayer,
                          currentDeal->hand[seat]);
    }

    while (currentDeal->currentTrick <= 13) {
        int current_fd = player_fds[currentDeal->currentPlayer];

        do {
            try {
                protocol.sendTRICK(current_fd,
                                   currentDeal->currentTrick,
                                   currentDeal->cardsOnTable);
            }
            catch (Error& e) {
                std::string message = e.what();
                // if (isSubstring(message, "would block")) {
                //     std::cerr << message << std::endl;
                // }
                // else {
                //     throw e;
                // }
                std::cerr << message << std::endl;
            }
            askedTRICK = current_fd;
        } while (!cv_TRICK.wait_for(lock,
                                    std::chrono::seconds(protocol.timeout),
                                    [this] { return askedTRICK == -1; }));

        currentDeal->nextPlayer();

        if (currentDeal->currentPlayer == currentDeal->firstPlayer) {
            // end of trick, handle score

            scores[currentDeal->highestPlayer] += currentDeal->getScore();

            for (const auto& [seat, fd] : player_fds) {
                protocol.sendTAKEN(fd,
                                   currentDeal->currentTrick,
                                   currentDeal->cardsOnTable,
                                   currentDeal->highestPlayer);
            }

            currentDeal->nextTrick();
        }
    }
}

std::string Server::handleIAM(int fd) {
    std::string seat;
    protocol.recvIAM(fd, seat);

    std::unique_lock<std::mutex> lock(mtx);

    if (player_fds.contains(seat)) {
        std::string activePlayers = getKeys(player_fds);
        lock.unlock();
        protocol.sendBUSY(fd, activePlayers);
        throw Error("seat " + seat + " already taken");
    }
    else {
        player_fds[seat] = fd;
        if (player_fds.size() == 4) {
            cv_allplayers.notify_one();
        }
        lock.unlock();
    }

    return seat;
}

void Server::handleTRICK(int fd) {
    uint8_t trick;
    std::string cardPlaced;
    protocol.recvTRICK(fd, &trick, cardPlaced);

    std::unique_lock<std::mutex> lock(mtx);

    if (askedTRICK == fd) {
        if (currentDeal->isLegal(trick, cardPlaced)) {
            // asked client sent legal TRICK
            currentDeal->playCard(cardPlaced);
            askedTRICK = -1;
            cv_TRICK.notify_one();
            lock.unlock();
        }
        else {
            // asked client sent illegal TRICK
            lock.unlock();
            protocol.sendWRONG(fd, trick);
        }
    }
    else {
        // unasked client sent TRICK
        lock.unlock();
        protocol.sendWRONG(fd, trick);
    }
}
