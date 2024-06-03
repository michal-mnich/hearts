#ifndef COMMON_H
#define COMMON_H

#include <map>
#include <string>

bool isSubstring(const std::string& s1, const std::string& s2);
std::string createTimestamp();
std::string createLog(const std::string& from,
                      const std::string& to,
                      const std::string& message);
std::string getRandomSeat();
std::string getKeys(const std::map<std::string, int>& map);

bool isRank(char c);
bool isSuit(char c);
std::string getPrettyCards(const std::string& cardString, bool sort = false);
std::string findCardWithSuit(const std::string& cards, char suit);
void deleteCard(std::string& cards, const std::string& card);
std::string getRandomCard(const std::string& cards);

// return rank(c1) < rank(c2)
bool compareRanks(const std::string& c1, const std::string& c2);

#endif // COMMON_H
