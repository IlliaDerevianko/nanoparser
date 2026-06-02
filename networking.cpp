#include "networking.hpp"

#include <iostream>
#include <cerrno>
#include <cstring>
#include <span>
#include <stdexcept>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

namespace nanoparser {
    UDPSender::UDPSender(const std::string_view multicast_ip, const uint16_t dst_port, const std::string_view src_ip)
        : sockfd_(-1)
        , addr_{}
    {
        sockfd_ = socket(AF_INET, SOCK_DGRAM, 0);

        if (sockfd_ < 0)
            throw std::runtime_error("Failed to create socket");

        in_addr local_if{};
        local_if.s_addr = inet_addr(src_ip.data());
        if (setsockopt(sockfd_, IPPROTO_IP, IP_MULTICAST_IF, &local_if, sizeof(local_if)) < 0) {
            std::cerr << "setsockopt failed with error: " << strerror(errno) << " (Code: " << errno << ")\n";
            throw std::runtime_error("Failed to set IP_MULTICAST_IF");
        }

        constexpr int ttl = 1;
        setsockopt(sockfd_, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));

        constexpr int loop = 0;
        setsockopt(sockfd_, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));

        addr_.sin_family = AF_INET;
        addr_.sin_addr.s_addr = inet_addr(multicast_ip.data());
        addr_.sin_port = htons(dst_port);
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

    void MockUDPSender::send([[maybe_unused]] std::span<const char> buf) {
        // no-op
    }
}


