#ifndef NETWORK_COMMON_H
#define NETWORK_COMMON_H

#include <arpa/inet.h>
#include <string>

#define QUEUE_SIZE 4

void _close(int sock_fd);
void _shutdown(int sock_fd, int how);
int _socket(int domain);
void _bind(int sock_fd, struct sockaddr* addr, socklen_t len);
void _listen(int sock_fd);
int _accept(int sock_fd);
void _getsockname(int sock_fd, struct sockaddr_storage* addr);
void _getpeername(int sock_fd, struct sockaddr_storage* addr);

bool _validAddress(struct addrinfo* hints, struct addrinfo* res);
std::string _domainToString(int domain);
std::string _getAddressString(struct sockaddr_storage* addr);

std::string getPeerAddress(int sock_fd);
std::string getLocalAddress(int sock_fd);

void waitPollIn(int fd, int timeout);
void waitPollOut(int fd);

std::string recvMessage(int fd, int timeout);
void sendMessage(int fd, const std::string& message);

void setNonBlocking(int fd);
void unsetNonBlocking(int fd);

#endif // NETWORK_COMMON_H
