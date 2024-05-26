#ifndef COMMON_H
#define COMMON_H

#include <string>

bool isSubstring(const std::string& s1, const std::string& s2);
std::string createTimestamp();
std::string createLog(std::string from, std::string to, std::string message);

#endif // COMMON_H
