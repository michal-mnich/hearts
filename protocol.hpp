#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <string>

class ServerProtocol {
private:
    unsigned int timeout;
public:
    ServerProtocol(unsigned int timeout);
    std::string recvIAM(int fd);
    void sendBUSY(int fd);
    void sendDEAL();
    void sendTRICK();
    void recvTRICK();
    void sendWRONG();
    void sendTAKEN();
    void sendSCORE();
    void sendTOTAL();
};

class ClientProtocol {
public:
    ClientProtocol();
    void sendIAM(int fd, std::string seat);
    void recvBUSY();
    void recvDEAL();
    void recvTRICK();
    void sendTRICK();
    void recvWRONG();
    void recvTAKEN();
    void recvSCORE();
    void recvTOTAL();
};

#endif // PROTOCOL_H
