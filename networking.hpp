#pragma once
#include <string>
#include <netinet/in.h>

namespace networking{
    constexpr size_t kMaxUdpPayload = 1472;
}


class UDPsocket {
public:
    UDPsocket(std::string ip_addr, uint16_t port);
    ssize_t send(const void* buf, size_t size);
    ~UDPsocket();

private:
    const std::string ip_addr_;
    const uint16_t port_;
    int sockfd_;
    sockaddr_in addr_;
};
