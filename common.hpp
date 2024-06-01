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
std::string getPrettyCards(const std::string& cardString);

class SimpleCV {
private:
    std::mutex mtx;
    std::condition_variable cv;
    bool notified = false;

public:
    SimpleCV();
    void notify();
    bool wait_for(unsigned int timeout);
};

class ReadersWriters {
private:
    std::mutex mtx;
    std::condition_variable cv;
    int reader_count = 0;
    bool writer_active = false;
    int waiting_writers = 0;

public:
    void startRead();
    void endRead();
    void startWrite();
    void endWrite();
};

#endif // COMMON_H
