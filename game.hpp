#ifndef GAME_H
#define GAME_H

#include <cstdint>
#include <map>
#include <semaphore>
#include <string>
#include <vector>

class Deal {
public:
    char trickSuit = 0;
    std::string highestCard;
    std::string highestPlayer;

    uint8_t type;
    std::string firstPlayer;
    std::map<std::string, std::string> originalHand; // seat, cards
    std::map<std::string, std::string> currentHand; // seat, cards
    std::map<std::string, unsigned int> scores; // player, score
    std::map<std::string, std::vector<std::string>> tricksTaken;

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
