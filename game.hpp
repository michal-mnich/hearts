#ifndef GAME_H
#define GAME_H

#include <cstdint>
#include <map>
#include <semaphore>
#include <string>

class Deal {
public:
    char trickColor = 0;
    std::string highestCard;
    std::string highestPlayer;

    uint8_t type;
    std::string firstPlayer;
    std::map<std::string, std::string> hand; // seat, cards

    std::string currentPlayer;
    uint8_t currentTrick;
    std::string cardsOnTable;

    void nextPlayer();
    bool isLegal(uint8_t trick, std::string& cardPlaced);
    void playCard(const std::string& card);
    unsigned int getScore();
    void nextTrick();
};

#endif // GAME_H
