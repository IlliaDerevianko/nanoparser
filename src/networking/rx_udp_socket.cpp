#include "rx_udp_socket.hpp"

#include <cstring>
#include <iostream>
#include <arpa/inet.h>
#include <linux/net_tstamp.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include "message_header.hpp"

namespace nanoparser {
    RxUdpSocket::RxUdpSocket(const std::string& dst_ip, const std::string& multicast_group_ip, uint16_t port) {
        // Enable port reuse for multicast connections.
        constexpr int reuse = 1;
        if (setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
            std::cerr << "setsockopt failed with error: " << strerror(errno) << " (Code: " << errno << ")\n";
            throw std::runtime_error("setsockopt SO_REUSEADDR failed");
        }
        // Bind to the receiving port.
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = INADDR_ANY;
        if (bind(sockfd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1) {
            std::cerr << "bind failed with error: " << strerror(errno) << "\n";
            throw std::runtime_error("failed to bind to the receiving port");
        }
        // Join the multicast group.
        ip_mreq mreq{};
        mreq.imr_multiaddr.s_addr = inet_addr(multicast_group_ip.c_str());
        mreq.imr_interface.s_addr = inet_addr(dst_ip.c_str());
        if (setsockopt(sockfd_, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) == -1)
            std::cerr << "IP_ADD_MEMBERSHIP failed: " << strerror(errno) << "\n";
    }

    void RxUdpSocket::EnableRxHwTimestamping(const std::string& dst_interface) const {
        // Enable RX hardware timestamping on the NIC.
        hwtstamp_config hwconfig{};
        hwconfig.tx_type = HWTSTAMP_TX_OFF;
        hwconfig.rx_filter = HWTSTAMP_FILTER_ALL;
        ifreq ifr{};
        std::strncpy(ifr.ifr_name, dst_interface.c_str(), IFNAMSIZ - 1);
        ifr.ifr_data = reinterpret_cast<char*>(&hwconfig);
        if (ioctl(sockfd_, SIOCSHWTSTAMP, &ifr) == -1) {
            std::cerr << "ioctl SIOCSHWTSTAMP failed with error: " << strerror(errno) << " (Code: " << errno << ")\n";
            throw std::runtime_error("Failed to enable RX hardware timestamping on NIC");
        }

        // Enable RX hardware timestamping on the socket.
        constexpr int ts_flags = SOF_TIMESTAMPING_RX_HARDWARE |
                             SOF_TIMESTAMPING_RAW_HARDWARE;

        if (setsockopt(sockfd_, SOL_SOCKET, SO_TIMESTAMPING, &ts_flags, sizeof(ts_flags)) == -1) {
            std::cerr << "setsockopt failed with error: " << strerror(errno) << " (Code: " << errno << ")\n";
            throw std::runtime_error("failed to set SO_TIMESTAMPING");
        }
    }

    void RxUdpSocket::SetReceiveTimeout(std::chrono::milliseconds timeout) const {
        if (timeout.count() < 0)
            throw std::invalid_argument("timeout argument must be non-negative");

        // Set timeout for the blocking receive syscall.
        auto secs = std::chrono::duration_cast<std::chrono::seconds>(timeout);
        auto usecs = std::chrono::duration_cast<std::chrono::microseconds>(timeout - secs);
        timeval tv{};
        tv.tv_sec = secs.count();
        tv.tv_usec = usecs.count();
        if (setsockopt(sockfd_, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv) == -1) {
            std::cerr << "setsockopt failed with error: " << strerror(errno) << " (Code: " << errno << ")\n";
            throw std::runtime_error("failed to set receive socket timeout");
        }
    }
}
