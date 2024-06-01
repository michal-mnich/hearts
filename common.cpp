#include "common.hpp"
#include <chrono>
#include <map>
#include <random>

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

std::string getPrettyCards(const std::string& cardString) {
    std::string result;
    std::string rank;
    char suit;

    for (size_t i = 0; i < cardString.size(); ++i) {
        if (std::isdigit(cardString[i])) {
            rank += cardString[i];
        }
        else {
            if (rank.empty() || rank == "1") {
                rank += cardString[i];
                continue;
            }
            suit = cardString[i];
            result += rank + suit;
            if (i != cardString.size() - 1) {
                result += ", ";
            }
            rank.clear();
        }
    }

    if (result.empty()) {
        result = "None";
    }

    return result;
}

void ReadersWriters::startRead() {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [this] { return !writer_active && waiting_writers == 0; });
    ++reader_count;
}

void ReadersWriters::endRead() {
    std::unique_lock<std::mutex> lock(mtx);
    --reader_count;
    if (reader_count == 0) {
        cv.notify_all();
    }
}

void ReadersWriters::startWrite() {
    std::unique_lock<std::mutex> lock(mtx);
    ++waiting_writers;
    cv.wait(lock, [this] { return reader_count == 0 && !writer_active; });
    --waiting_writers;
    writer_active = true;
}

void ReadersWriters::endWrite() {
    std::unique_lock<std::mutex> lock(mtx);
    writer_active = false;
    cv.notify_all();
}
