#ifndef GAME_H
#define GAME_H

#include <cstdint>
#include <map>
#include <semaphore>
#include <string>

class Deal {
public:
    uint8_t type;
    std::string firstPlayer;
    std::map<std::string, std::string> hand; // seat, cards

    std::string currentPlayer;
    uint8_t currentTrick;
    std::string cardsOnTable;

    void nextPlayer();
    Deal();
};

#endif // GAME_H
