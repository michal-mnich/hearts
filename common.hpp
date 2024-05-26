#ifndef COMMON_H
#define COMMON_H

#include <string>
#include <map>

bool isSubstring(const std::string& s1, const std::string& s2);
std::string createTimestamp();
std::string createLog(std::string from, std::string to, std::string message);
std::string getRandomSeat();
std::string getKeys(std::map<std::string, int>& map);

#endif // COMMON_H
