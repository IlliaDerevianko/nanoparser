#include "tx_udp_socket.hpp"

#include <cstring>
#include <iostream>
#include <arpa/inet.h>
#include <linux/net_tstamp.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>

namespace nanoparser {
    TxUdpSocket::TxUdpSocket(const std::string& src_ip, const std::string& multicast_group_ip, uint16_t dst_port) {
        // Select a specific IP address to send packets from.
        in_addr local_if{};
        local_if.s_addr = inet_addr(src_ip.c_str());
        if (setsockopt(sockfd_, IPPROTO_IP, IP_MULTICAST_IF, &local_if, sizeof(local_if)) == -1) {
            std::cerr << "setsockopt failed with error: " << strerror(errno) << " (Code: " << errno << ")\n";
            throw std::runtime_error("setsockopt IP_MULTICAST_IF failed");
        }
        // Ensure the packet doesn't leave the subnet.
        constexpr int ttl = 1;
        if (setsockopt(sockfd_, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) == -1) {
            std::cerr << "setsockopt failed with error: " << strerror(errno) << " (Code: " << errno << ")\n";
            throw std::runtime_error("setsockopt IP_MULTICAST_TTL failed");
        }
        // Disable echoing.
        constexpr int loop = 0;
        if (setsockopt(sockfd_, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop)) == -1) {
            std::cerr << "setsockopt failed with error: " << strerror(errno) << " (Code: " << errno << ")\n";
            throw std::runtime_error("setsockopt IP_MULTICAST_LOOP failed");
        }

        // Connect the UDP socket to delegate address management to the kernel.
        sockaddr_in addr_{};
        addr_.sin_family = AF_INET;
        addr_.sin_addr.s_addr = inet_addr(multicast_group_ip.c_str());
        addr_.sin_port = htons(dst_port);
        if (connect(sockfd_, reinterpret_cast<sockaddr*>(&addr_), sizeof(addr_)) == -1) {
            std::cerr << "socket connection failed with error: " << strerror(errno) << " (Code: " << errno << ")\n";
            throw std::runtime_error("connecting a socket failed");
        }
    }

    void TxUdpSocket::Send(std::span<const char> buf) const {
        ssize_t bytes = send(sockfd_, buf.data(), buf.size(), 0);
        if (bytes == -1) [[unlikely]] {
            std::cerr << "socket send failed with error: " << strerror(errno) << " (Code: " << errno << ")\n";
            throw std::runtime_error("socket send failed");
        }
    }

    void TxUdpSocket::EnableTxHwTimestamping(const std::string& src_interface) const {
        // Enable TX hardware timestamping on the NIC.
        hwtstamp_config hwconfig{};
        hwconfig.tx_type = HWTSTAMP_TX_ON;
        hwconfig.rx_filter = HWTSTAMP_FILTER_NONE;
        ifreq ifr{};
        std::strncpy(ifr.ifr_name, src_interface.c_str(), IFNAMSIZ - 1);
        ifr.ifr_data = reinterpret_cast<char*>(&hwconfig);
        if (ioctl(sockfd_, SIOCSHWTSTAMP, &ifr) == -1) {
            std::cerr << "ioctl SIOCSHWTSTAMP failed with error: " << strerror(errno) << " (Code: " << errno << ")\n";
            throw std::runtime_error("failed to enable TX hardware timestamping on NIC");
        }

        // Enable TX hardware timestamping on the socket.
        constexpr int ts_flags = SOF_TIMESTAMPING_TX_HARDWARE |
                                 SOF_TIMESTAMPING_RAW_HARDWARE |
                                 SOF_TIMESTAMPING_OPT_TSONLY;

        if (setsockopt(sockfd_, SOL_SOCKET, SO_TIMESTAMPING, &ts_flags, sizeof(ts_flags)) == -1) {
            std::cerr << "setsockopt failed with error: " << strerror(errno) << " (Code: " << errno << ")\n";
            throw std::runtime_error("failed to set SO_TIMESTAMPING");
        }
    }
}
