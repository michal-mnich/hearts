#include "server.hpp"
#include "error.hpp"
#include "network_common.hpp"
#include "protocol_server.hpp"
#include <fstream>
#include <iostream>

/*
    nieblokujące wysyłanie przyjmujące mutex jako argument,
    można czekać na condition_variable z timeoutem

    przedwczesne zakończenie rozdania, gdy punkty zostały rozdane

    przetestować rozłączanie się klientów w różnych scenariuszach,
    sprawdzić czy 'tricks' 'cards' SCORE i TOTAL są poprawne

    sprawdzić czy są poprawne kody wyjścia serwera (i klienta)

    ulepszyć heurystykę wyboru karty do zagrania w kliencie

    obsługa wyjątków w wątkach (propagowanie wyjątków między threadami?)
*/

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
      gameOver(false) {
    parseFile(config.file);
}

void Server::start() {
    debug("Starting accept thread...");
    auto accept_thread = std::thread(&Server::acceptThread, this);

    for (size_t i = 0; i < deals.size(); i++) {
        debug("Starting deal " + std::to_string(i + 1) + "...");
        currentDeal = &deals[i];

        debug("Starting deal thread...");
        auto deal_thread = std::thread(&Server::dealThread, this);
        deal_thread.join();

        debug("Deal " + std::to_string(i + 1) + " finished!");
    }

    debug("Game over!");
    gameOver.store(true);

    networker.stopAccepting();
    accept_thread.join();
    debug("Accept thread stopped!");

    networker.disconnectAll();
    networker.joinClients();
    debug("All clients threads stopped!");
}

void Server::acceptThread() {
    networker.startAccepting(this);
    if (!gameOver.load()) throw Error("unexpectedly stopped accepting clients");
}

void Server::playerThread(int fd) {
    std::string seat;

    try {
        seat = handleIAM(fd);
    }
    catch (Error& e) {
        if (!gameOver.load()) std::cerr << e.what() << std::endl;
        networker.disconnectOne(fd);
        return;
    }

    while (true) {
        try {
            handleTRICK(fd);
        }
        catch (Error& e) {
            std::lock_guard<std::mutex> lock(mtx);
            if (!gameOver.load()) {
                std::cerr << e.what() << std::endl;
                if (dealStarted) dealSuspended = true;
            }
            player_fds.erase(seat);
            networker.disconnectOne(fd);
            return;
        }
    }
}

void Server::dealThread() {
    std::unique_lock<std::mutex> lock(mtx);
    cvReady.wait(lock, [this] { return player_fds.size() == 4; });
    dealStarted = true;

    debug("All players are ready!");

    for (const auto& [seat, fd] : player_fds) {
        protocol.sendDEAL(fd,
                          currentDeal->type,
                          currentDeal->firstPlayer,
                          currentDeal->originalHand[seat],
                          &lock);
    }

    while (currentDeal->currentTrick <= 13) {
        do {
            trickDone[currentDeal->currentPlayer] = false;

            if (dealSuspended) {
                debug("Suspending game thread...");
                cvSuspended.wait(lock, [this] { return !dealSuspended; });
                debug("Resuming game thread...");
            }

            if (trickDone[currentDeal->currentPlayer]) {
                debug("Got valid trick from current player during suspension!");
                break;
            }

            fdExpectedTrick = player_fds[currentDeal->currentPlayer];

            try {
                protocol.sendTRICK(fdExpectedTrick,
                                   currentDeal->currentTrick,
                                   currentDeal->cardsOnTable,
                                   &lock);
            }
            catch (Error& e) {
                std::cerr << e.what() << std::endl;
            }

        } while (!cvTrick.wait_for(
            lock,
            std::chrono::seconds(protocol.timeout),
            [this] { return trickDone[currentDeal->currentPlayer]; }));

        currentDeal->nextPlayer();

        if (currentDeal->currentPlayer == currentDeal->firstPlayer) {
            for (const auto& [seat, fd] : player_fds) {
                protocol.sendTAKEN(fd,
                                   currentDeal->currentTrick,
                                   currentDeal->cardsOnTable,
                                   currentDeal->highestPlayer,
                                   &lock);
            }
            currentDeal->nextTrick();
        }
    }

    for (const auto& [seat, fd] : player_fds) {
        totalScores[seat] += currentDeal->scores[seat];
    }

    for (const auto& [seat, fd] : player_fds) {
        protocol.sendSCORE(fd, currentDeal->scores, &lock);
        protocol.sendTOTAL(fd, totalScores, &lock);
    }

    dealStarted = false;
}

std::string Server::handleIAM(int fd) {
    std::string seat;
    protocol.recvIAM(fd, seat);

    std::unique_lock<std::mutex> lock(mtx);

    if (player_fds.contains(seat)) {
        std::string activePlayers = getKeys(player_fds);
        protocol.sendBUSY(fd, activePlayers, &lock);
        throw Error("seat " + seat + " already taken");
    }
    else {
        player_fds[seat] = fd;
        if (dealSuspended) {
            debug("Remaining client arrived!");
            protocol.sendDEAL(fd,
                              currentDeal->type,
                              currentDeal->firstPlayer,
                              currentDeal->originalHand[seat],
                              &lock);
            for (size_t i = 0; i < currentDeal->tricksTaken.size(); i++) {
                auto [cardsTaken, highestPlayer] = currentDeal->tricksTaken[i];
                protocol.sendTAKEN(fd, i + 1, cardsTaken, highestPlayer, &lock);
            }
            if (seat == currentDeal->currentPlayer) {
                trickDone[seat] = false;
                fdExpectedTrick = fd;
                protocol.sendTRICK(fd,
                                   currentDeal->currentTrick,
                                   currentDeal->cardsOnTable,
                                   &lock);
            }
            dealSuspended = false;
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
    if (dealSuspended) {
        debug("Suspending handle TRICK...");
        cvSuspended.wait(lock, [this] { return !dealSuspended; });
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
            trickDone[currentDeal->currentPlayer] = true;
            fdExpectedTrick = -1;
            cvTrick.notify_one();
        }
        else {
            // asked client sent illegal TRICK
            protocol.sendWRONG(fd, currentDeal->currentTrick, &lock);
        }
    }
    else {
        // unasked client sent TRICK
        protocol.sendWRONG(fd, currentDeal->currentTrick, &lock);
    }
}
