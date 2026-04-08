#include "networking.hpp"

#include <utility>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>


UDPsocket::UDPsocket(std::string ip_addr, const uint16_t port)
    : ip_addr_(std::move(ip_addr)), port_(port), addr_{} {

    sockfd_ = socket(AF_INET, SOCK_DGRAM, 0);

    in_addr local_if{};
    local_if.s_addr = htonl(INADDR_LOOPBACK);
    setsockopt(sockfd_, IPPROTO_IP, IP_MULTICAST_IF, &local_if, sizeof(local_if));

    constexpr int ttl = 1;
    setsockopt(sockfd_, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));

    constexpr int loop = 1;
    setsockopt(sockfd_, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));

    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr = inet_addr(ip_addr_.c_str());
    addr_.sin_port = htons(port);
}


ssize_t UDPsocket::send(const void* buf, const size_t size) {
    return sendto(sockfd_, buf, size, 0, reinterpret_cast<sockaddr*>(&addr_), sizeof(addr_));
}


UDPsocket::~UDPsocket() {
    if (sockfd_ >= 0)
        close(sockfd_);
}

