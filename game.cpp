#include "game.hpp"
#include "error.hpp"
#include "common.hpp"
#include <algorithm>
#include <vector>

static std::vector<std::string> playerOrder = {"N", "E", "S", "W"};

void Deal::nextPlayer() {
    auto it = std::find(playerOrder.begin(), playerOrder.end(), currentPlayer);
    it++;
    if (it == playerOrder.end()) it = playerOrder.begin();
    currentPlayer = *it;
}

bool Deal::isLegal(uint8_t trick, std::string& cardPlaced) {
    if (trick != currentTrick) return false;
    if (cardsOnTable.find(cardPlaced) != std::string::npos) return false;
    if (hand[currentPlayer].find(cardPlaced) == std::string::npos) return false;
    if (cardsOnTable.empty()) return true;
    char tableSuit = cardsOnTable.back();
    char placedSuit = cardPlaced.back();
    if (tableSuit == placedSuit) return true;
    if (hand[currentPlayer].find(tableSuit) != std::string::npos) return false;
    return true;
}

void Deal::playCard(const std::string& card) {
    deleteCard(hand[currentPlayer], card);

    if (cardsOnTable.empty()) trickColor = card.back();

    if (trickColor == card.back()) {
        if (highestCard.empty() || compareRanks(highestCard, card)) {
            highestCard = card;
            highestPlayer = currentPlayer;
        }
    }

    cardsOnTable.append(card);
}

unsigned int Deal::getScore() {
    auto b = cardsOnTable.begin();
    auto e = cardsOnTable.end();
    switch (type) {
        case 1:
            return 1;
        case 2:
            return std::count(b, e, 'H');
        case 3:
            return std::count(b, e, 'Q') * 5;
        case 4:
            return (std::count(b, e, 'J') + std::count(b, e, 'K')) * 2;
        case 5:
            return (cardsOnTable.find("KH") != std::string::npos) * 18;
        case 6:
            return (currentTrick == 7 || currentTrick == 13) * 10;
        case 7:
            unsigned int score = 0;
            score += 1;
            score += std::count(b, e, 'H');
            score += std::count(b, e, 'Q') * 5;
            score += (std::count(b, e, 'J') + std::count(b, e, 'K')) * 2;
            score += (cardsOnTable.find("KH") != std::string::npos) * 18;
            score += (currentTrick == 7 || currentTrick == 13) * 10;
            return score;
    }
    throw Error("unexpected type: " + std::to_string(type));
}

void Deal::nextTrick() {
    currentTrick++;

    currentPlayer = highestPlayer;
    firstPlayer = highestPlayer;

    hand[highestPlayer] += cardsOnTable;

    cardsOnTable.clear();
    highestCard.clear();
    highestPlayer.clear();
    trickColor = 0;
}
