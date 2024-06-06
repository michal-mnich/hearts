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
        goto disconnect;
    }

    while (true) {
        try {
            handleTRICK(fd);
        }
        catch (Error& e) {
            if (!game_over.load()) std::cerr << e.what() << std::endl;
            goto disconnect;
        }
    }

disconnect:
    player_fds.erase(seat);
    networker.disconnectOne(fd);
}

void Server::gameThread() {
    std::unique_lock<std::mutex> lockRunning(mtxRunning);
    cvRunning.wait(lockRunning, [this] { return player_fds.size() == 4; });

    debug("All players are ready!");

    for (const auto& [seat, fd] : player_fds) {
        protocol.sendDEAL(fd,
                          currentDeal->type,
                          currentDeal->firstPlayer,
                          currentDeal->originalHand[seat]);
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
        } while (!cvTrick.wait_for(lockRunning,
                                    std::chrono::seconds(protocol.timeout),
                                    [this] { return askedTRICK == -1; }));

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

    std::unique_lock<std::mutex> lockRunning(mtxRunning);

    if (player_fds.contains(seat)) {
        std::string activePlayers = getKeys(player_fds);
        lockRunning.unlock();
        protocol.sendBUSY(fd, activePlayers);
        throw Error("seat " + seat + " already taken");
    }
    else {
        player_fds[seat] = fd;
        if (player_fds.size() == 4) {
            cvRunning.notify_one();
        }
        lockRunning.unlock();
    }

    return seat;
}

void Server::handleTRICK(int fd) {
    uint8_t trick;
    std::string cardPlaced;
    protocol.recvTRICK(fd, trick, cardPlaced);

    std::unique_lock<std::mutex> lockRunning(mtxRunning);

    if (askedTRICK == fd) {
        if (currentDeal->isLegal(trick, cardPlaced)) {
            // asked client sent legal TRICK
            currentDeal->playCard(cardPlaced);
            askedTRICK = -1;
            cvTrick.notify_one();
            lockRunning.unlock();
        }
        else {
            // asked client sent illegal TRICK
            lockRunning.unlock();
            protocol.sendWRONG(fd, currentDeal->currentTrick);
        }
    }
    else {
        // unasked client sent TRICK
        lockRunning.unlock();
        protocol.sendWRONG(fd, currentDeal->currentTrick);
    }
}
