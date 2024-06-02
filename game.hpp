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
    uint8_t currentTrick = 0;
    std::string cardsOnTable;

    void nextPlayer();
    bool isLegal(uint8_t trick, std::string& cardPlaced);
    void playCard(std::string& card);
    unsigned int getScore();
};

#endif // GAME_H
