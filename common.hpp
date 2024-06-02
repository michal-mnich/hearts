#ifndef COMMON_H
#define COMMON_H

#include <chrono>
#include <condition_variable>
#include <map>
#include <mutex>
#include <string>

bool isSubstring(const std::string& s1, const std::string& s2);
std::string createTimestamp();
std::string createLog(std::string from, std::string to, std::string message);
std::string getRandomSeat();
std::string getKeys(std::map<std::string, int>& map);

bool isRank(char c);
bool isSuit(char c);
std::string getPrettyCards(const std::string& cardString, bool sort = false);
std::string findCardWithSuit(const std::string& cards, char suit);
void deleteCard(std::string& cards, const std::string& card);
std::string getRandomCard(const std::string& cards);

#endif // COMMON_H
