#include "game.hpp"
#include "common.hpp"
#include "error.hpp"
#include <algorithm>
#include <vector>

static std::vector<std::string> playerOrder = {"N", "E", "S", "W"};

void Deal::nextPlayer() {
    auto it = std::find(playerOrder.begin(), playerOrder.end(), currentPlayer);
    it++;
    if (it == playerOrder.end()) it = playerOrder.begin();
    currentPlayer = *it;
}

bool Deal::isLegal(uint8_t trick, std::string& card) {
    if (trick != currentTrick) {
        // wrong trick number
        return false;
    }
    if (cardsOnTable.find(card) != std::string::npos) {
        // card already on table
        return false;
    }
    if (currentHand[currentPlayer].find(card) == std::string::npos) {
        // player doesn't have card
        return false;
    }
    if (cardsOnTable.empty()) {
        // first card in trick
        return true;
    }
    if (card.back() == trickSuit) {
        // player played trick suit
        return true;
    }
    if (currentHand[currentPlayer].find(trickSuit) != std::string::npos) {
        // player has trick suit, but didn't play it
        return false;
    }
    return true;
}

void Deal::playCard(const std::string& card) {
    deleteCard(currentHand[currentPlayer], card);

    if (cardsOnTable.empty()) trickSuit = card.back();

    if (card.back() == trickSuit) {
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

// bool Deal::isOver() {
//     auto b = cardsOnTable.begin();
//     auto e = cardsOnTable.end();
//     switch (type) {
//         case 1:
//             return false;
//         case 2:
//             for (auto& [s, h] : currentHand) {
//                 if (std::find(h.begin(), h.end(), 'H') != h.end()) {
//                     return false;
//                 }
//             }
//             return true;
//         case 3:
//             return std::count(b, e, 'Q') * 5;
//         case 4:
//             return (std::count(b, e, 'J') + std::count(b, e, 'K')) * 2;
//         case 5:
//             return (cardsOnTable.find("KH") != std::string::npos) * 18;
//         case 6:
//             return (currentTrick == 7 || currentTrick == 13) * 10;
//         case 7:
//             unsigned int score = 0;
//             score += 1;
//             score += std::count(b, e, 'H');
//             score += std::count(b, e, 'Q') * 5;
//             score += (std::count(b, e, 'J') + std::count(b, e, 'K')) * 2;
//             score += (cardsOnTable.find("KH") != std::string::npos) * 18;
//             score += (currentTrick == 7 || currentTrick == 13) * 10;
//             return score;
//     }
//     throw Error("unexpected type: " + std::to_string(type));
// }

void Deal::nextTrick() {
    scores[highestPlayer] += getScore();
    tricksTaken.emplace_back(cardsOnTable, highestPlayer);

    currentTrick++;

    currentPlayer = highestPlayer;
    firstPlayer = highestPlayer;

    cardsOnTable.clear();
    highestCard.clear();
    highestPlayer.clear();
    trickSuit = 0;
}
