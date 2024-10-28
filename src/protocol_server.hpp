#ifndef PROTOCOL_SERVER_H
#define PROTOCOL_SERVER_H

#include "network_server.hpp"
#include <map>
#include <string>
#include <mutex>

class ServerProtocol {
private:
    ServerNetworker* networker;

public:
    void logMessage(int client_fd, const std::string& message, bool incoming);

    unsigned int timeout;
    ServerProtocol(ServerNetworker* networker, unsigned int timeout);

    void recvIAM(int fd, std::string& seat);
    void sendBUSY(int fd,
                  const std::string& busySeats,
                  std::unique_lock<std::mutex>* lock = nullptr);
    void sendDEAL(int fd,
                  uint8_t type,
                  const std::string& first,
                  const std::string& cards,
                  std::unique_lock<std::mutex>* lock = nullptr);
    void sendTRICK(int fd,
                   uint8_t trick,
                   const std::string& cardsOnTable,
                   std::unique_lock<std::mutex>* lock = nullptr);
    void recvTRICK(int fd, uint8_t& trick, std::string& cardPlaced);
    void sendWRONG(int fd,
                   uint8_t trick,
                   std::unique_lock<std::mutex>* lock = nullptr);
    void sendTAKEN(int fd,
                   uint8_t trick,
                   const std::string& cardsTaken,
                   const std::string& seat,
                   std::unique_lock<std::mutex>* lock = nullptr);
    void sendSCORE(int fd,
                   std::map<std::string, unsigned int>& scores,
                   std::unique_lock<std::mutex>* lock = nullptr);
    void sendTOTAL(int fd,
                   std::map<std::string, unsigned int>& totals,
                   std::unique_lock<std::mutex>* lock = nullptr);

    bool tryParseIAM(const std::string& message, std::string& seat);
    bool tryParseTRICK(const std::string& message,
                       uint8_t& trick,
                       std::string& cardPlaced);
};

#endif // PROTOCOL_SERVER_H
