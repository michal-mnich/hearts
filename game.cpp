#include "game.hpp"
#include <algorithm>
#include <vector>

void Deal::nextPlayer() {
    std::vector<std::string> order = {"N", "E", "S", "W"};
    auto it = std::find(order.begin(), order.end(), currentPlayer);
    it++;
    if (it == order.end()) it = order.begin();
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

void Deal::playCard(std::string& card) {
    size_t pos = hand[currentPlayer].find(card);
    hand[currentPlayer].erase(pos, card.size());
    cardsOnTable.append(card);
}
