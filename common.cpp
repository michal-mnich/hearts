#include "common.hpp"
#include <chrono>
#include <random>
#include <map>

bool isSubstring(const std::string& s1, const std::string& s2) {
    return s1.find(s2) != std::string::npos;
}

std::string createTimestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm;
    localtime_r(&now_time_t, &now_tm);
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
                            now.time_since_epoch()) %
                        1000;
    std::ostringstream oss;
    oss << std::put_time(&now_tm, "%Y-%m-%dT%H:%M:%S.");
    oss << std::setw(3) << std::setfill('0') << milliseconds.count();
    return oss.str();
}

std::string createFilename(std::string name) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(100000, 999999);
    std::string id = std::to_string(distr(gen));

    std::string ts = createTimestamp();
    std::replace(ts.begin(), ts.end(), '-', '_');
    std::replace(ts.begin(), ts.end(), ':', '_');
    std::replace(ts.begin(), ts.end(), '.', '_');

    return ts + "_" + name + "_" + id + ".log";
}

std::string createLog(std::string from, std::string to, std::string message) {
    return "[" + from + "," + to + "," + createTimestamp() + "] " + message;
}

std::string getRandomSeat() {
    std::string seats[] = {"N", "E", "S", "W", "X"};

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(0, 4);

    return seats[distr(gen)];
}

std::string getKeys(std::map<std::string, int>& map) {
    std::string keys;
    for (auto const& [key, val] : map) {
        keys += key;
    }
    return keys;
}
