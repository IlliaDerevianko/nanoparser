#include "udp_socket.hpp"

#include <cstring>
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>

namespace nanoparser {
    UdpSocket::UdpSocket() {
        // Create an IP/UDP socket.
        sockfd_ = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd_ == -1) {
            std::cerr << "socket creation failed: " << strerror(errno) << " (Code: " << errno << ")\n";
            throw std::runtime_error("socket creation failed");
        }
    }

    UdpSocket::UdpSocket(UdpSocket&& other) noexcept : sockfd_(other.sockfd_) {
        other.sockfd_ = -1;
    }

    UdpSocket& UdpSocket::operator=(UdpSocket&& other) noexcept {
        if (this != &other) {
            if (sockfd_ >= 0) close(sockfd_);
            sockfd_ = other.sockfd_;
            other.sockfd_ = -1;
        }
        return *this;
    }

    UdpSocket::~UdpSocket() {
        if (sockfd_ >= 0)
            close(sockfd_);
    }

    int UdpSocket::Fd() const noexcept {
        return sockfd_;
    }
}

