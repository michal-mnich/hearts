#include "game.hpp"
#include <algorithm>
#include <vector>

Deal::Deal() : currentTrick(0) {}

void Deal::nextPlayer() {
    std::vector<std::string> order = {"N", "E", "S", "W"};
    auto it = std::find(order.begin(), order.end(), currentPlayer);
    it++;
    if (it == order.end()) it = order.begin();
    currentPlayer = *it;
}
