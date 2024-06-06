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
        deal.currentHand["N"] = line;

        std::getline(file, line);
        deal.currentHand["E"] = line;

        std::getline(file, line);
        deal.currentHand["S"] = line;

        std::getline(file, line);
        deal.currentHand["W"] = line;

        deal.originalHand = deal.currentHand;

        deals.push_back(deal);
    }
}

Server::Server(ServerConfig& config)
    : networker(config.port), protocol(&networker, config.timeout),
      game_over(false) {
    parseFile(config.file);
}

void Server::start() {
    debug("Starting accept thread...");
    auto accept_thread = std::thread(&Server::acceptThread, this);

    for (unsigned int i = 0; i < deals.size(); i++) {
        debug("Starting deal " + std::to_string(i + 1) + "...");
        currentDeal = &deals[i];

        debug("Starting game thread...");
        auto game_thread = std::thread(&Server::gameThread, this);
        game_thread.join();

        debug("Deal " + std::to_string(i + 1) + " finished!");
    }

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

    try {
        seat = handleIAM(fd);
    }
    catch (Error& e) {
        if (!game_over.load()) std::cerr << e.what() << std::endl;
        networker.disconnectOne(fd);
        return;
    }

    while (true) {
        try {
            handleTRICK(fd);
        }
        catch (Error& e) {
            std::lock_guard<std::mutex> lock(mtx);
            if (!game_over.load()) {
                std::cerr << e.what() << std::endl;
                isSuspended = true;
            }
            player_fds.erase(seat);
            networker.disconnectOne(fd);
            return;
        }
    }
}

void Server::gameThread() {
    std::unique_lock<std::mutex> lock(mtx);
    cvReady.wait(lock, [this] { return player_fds.size() == 4; });

    debug("All players are ready!");

    for (const auto& [seat, fd] : player_fds) {
        protocol.sendDEAL(fd,
                          currentDeal->type,
                          currentDeal->firstPlayer,
                          currentDeal->originalHand[seat]);
    }

    while (currentDeal->currentTrick <= 13) {
        do {
            if (isSuspended) {
                debug("Suspending game thread...");
                cvSuspended.wait(lock, [this] { return !isSuspended; });
                debug("Resuming game thread...");
                // continue;
            }

            fdExpectedTrick = player_fds[currentDeal->currentPlayer];

            try {
                protocol.sendTRICK(fdExpectedTrick,
                                   currentDeal->currentTrick,
                                   currentDeal->cardsOnTable);
            }
            catch (Error& e) {
                std::cerr << e.what() << std::endl;
            }

        } while (!cvTrick.wait_for(lock,
                                   std::chrono::seconds(protocol.timeout),
                                   [this] { return fdExpectedTrick == -1; }));

        currentDeal->nextPlayer();

        if (currentDeal->currentPlayer == currentDeal->firstPlayer) {
            for (const auto& [seat, fd] : player_fds) {
                protocol.sendTAKEN(fd,
                                   currentDeal->currentTrick,
                                   currentDeal->cardsOnTable,
                                   currentDeal->highestPlayer);
            }
            currentDeal->nextTrick();
        }
    }

    for (const auto& [seat, fd] : player_fds) {
        totalScores[seat] += currentDeal->scores[seat];
    }

    for (const auto& [seat, fd] : player_fds) {
        protocol.sendSCORE(fd, currentDeal->scores);
        protocol.sendTOTAL(fd, totalScores);
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
        if (isSuspended) {
            debug("Remaining client arrived!");
            protocol.sendDEAL(fd,
                              currentDeal->type,
                              currentDeal->firstPlayer,
                              currentDeal->originalHand[seat]);
            for (size_t i = 0; i < currentDeal->tricksTaken.size(); i++) {
                auto [cardsTaken, highestPlayer] = currentDeal->tricksTaken[i];
                protocol.sendTAKEN(fd, i + 1, cardsTaken, highestPlayer);
            }
            isSuspended = false;
            cvSuspended.notify_all();
        }
        else if (player_fds.size() == 4) {
            cvReady.notify_all();
        }
    }

    return seat;
}

void Server::handleTRICK(int fd) {
    auto message = recvMessage(fd, -1);

    std::unique_lock<std::mutex> lock(mtx);
    if (isSuspended) {
        debug("Suspending handle TRICK...");
        cvSuspended.wait(lock, [this] { return !isSuspended; });
        debug("Resuming handle TRICK...");
    }

    uint8_t trick;
    std::string cardPlaced;
    protocol.logMessage(fd, message, true);
    if (!protocol.tryParseTRICK(message, trick, cardPlaced)) {
        throw Error("invalid TRICK message");
    }

    if (fdExpectedTrick == fd) {
        if (currentDeal->isLegal(trick, cardPlaced)) {
            // asked client sent legal TRICK
            currentDeal->playCard(cardPlaced);
            fdExpectedTrick = -1;
            cvTrick.notify_one();
        }
        else {
            // asked client sent illegal TRICK
            lock.unlock();
            protocol.sendWRONG(fd, currentDeal->currentTrick);
        }
    }
    else {
        // unasked client sent TRICK
        lock.unlock();
        protocol.sendWRONG(fd, currentDeal->currentTrick);
    }
}
