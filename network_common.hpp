#ifndef NETWORK_COMMON_H
#define NETWORK_COMMON_H

#include <arpa/inet.h>
#include <mutex>
#include <string>

#define QUEUE_SIZE 4

void _close(int sock_fd);
void _shutdown(int sock_fd, int how);
int _socket(int domain);
void _bind(int sock_fd, struct sockaddr* addr, socklen_t len);
void _listen(int sock_fd);
int _accept(int sock_fd);

bool _validAddress(const struct addrinfo* hints, struct addrinfo* res);
std::string _domainToString(int domain);

std::string getPeerAddress(int sock_fd);
std::string getLocalAddress(int sock_fd);

void waitPollIn(int fd, int timeout);
void waitPollOut(int fd);

std::string recvMessage(int fd, int timeout);
void sendMessage(int fd, const std::string& message);

// void setNonBlocking(int fd);
// void unsetNonBlocking(int fd);

template <typename SendFn, typename... Args>
std::unique_lock<std::mutex> retrySend(std::mutex& mtx,
                                       unsigned int timeout,
                                       SendFn&& sendFn,
                                       int fd,
                                       Args&&... args);

#endif // NETWORK_COMMON_H
