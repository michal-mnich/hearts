#ifndef COMMON_H
#define COMMON_H

#include <string>
#include <map>
#include <mutex>
#include <condition_variable>
#include <chrono>

bool isSubstring(const std::string& s1, const std::string& s2);
std::string createTimestamp();
std::string createLog(std::string from, std::string to, std::string message);
std::string getRandomSeat();
std::string getKeys(std::map<std::string, int>& map);
std::string getPrettyCards(const std::string& cardString);

class SimpleCV {
private:
    std::mutex mtx;
    std::condition_variable cv;
    bool notified;

public:
    SimpleCV() : notified(false) {}

    void notify() {
        std::unique_lock<std::mutex> lock(mtx);
        notified = true;
        cv.notify_one();
    }

    bool wait_for(unsigned int timeout) {
        auto duration = std::chrono::seconds(timeout);
        std::unique_lock<std::mutex> lock(mtx);
        if (cv.wait_for(lock, duration, [this] { return notified; })) {
            notified = false;
            return true;
        }
        return false;
    }
};

#endif // COMMON_H
