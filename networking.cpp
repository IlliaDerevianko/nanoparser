#include "networking.hpp"

#include <span>
#include <stdexcept>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

namespace nanoparser {
    UDPSender::UDPSender(const std::string_view ip_addr, const uint16_t port) : sockfd_(-1), addr_{} {

        sockfd_ = socket(AF_INET, SOCK_DGRAM, 0);

        if (sockfd_ < 0)
            throw std::runtime_error("Failed to create socket");

        in_addr local_if{};
        local_if.s_addr = htonl(INADDR_LOOPBACK);
        setsockopt(sockfd_, IPPROTO_IP, IP_MULTICAST_IF, &local_if, sizeof(local_if));

        constexpr int ttl = 1;
        setsockopt(sockfd_, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));

        constexpr int loop = 1;
        setsockopt(sockfd_, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));

        addr_.sin_family = AF_INET;
        addr_.sin_addr.s_addr = inet_addr(ip_addr.data());
        addr_.sin_port = htons(port);
    }

    UDPSender::UDPSender(UDPSender&& other) noexcept
        : sockfd_(other.sockfd_)
        , addr_(other.addr_)
    {
        other.sockfd_ = -1;
    }

    UDPSender &UDPSender::operator=(UDPSender&& other) noexcept {
        if (this != &other) {
            if (sockfd_ >= 0) close(sockfd_);
            sockfd_ = other.sockfd_;
            addr_ = other.addr_;
            other.sockfd_ = -1;
        }
        return *this;
    }

    UDPSender::~UDPSender() {
        if (sockfd_ >= 0)
            close(sockfd_);
    }

    ssize_t UDPSender::send(std::span<const char> buf) {
        return sendto(
            sockfd_,
            buf.data(),
            buf.size(),
            0,
            reinterpret_cast<sockaddr*>(&addr_),
            sizeof(addr_));
    }
}


