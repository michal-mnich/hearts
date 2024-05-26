#ifndef NETWORK_COMMON_H
#define NETWORK_COMMON_H

#include <arpa/inet.h>
#include <string>

#define MAX_CLIENTS 4
#define QUEUE_SIZE  4

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

void readn(int fd, void* vptr, size_t n);
void writen(int fd, const void* vptr, size_t n);

void socket_set_timeout(int socket_fd, unsigned int timeout);
void socket_clear_timeout(int socket_fd);

std::string readUntilEnd(int fd);

#endif // NETWORK_COMMON_H
