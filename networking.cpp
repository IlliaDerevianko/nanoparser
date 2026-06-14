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
#include <net/if.h>
#include <linux/net_tstamp.h>
#include <sys/ioctl.h>
#include <linux/errqueue.h>
#include <poll.h>

#include "constants.hpp"
#include "time.hpp"

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

        // --- 2. Wake Up the NIC Hardware Clock ---
        hwtstamp_config hwconfig{};
        hwconfig.tx_type = HWTSTAMP_TX_ON;
        hwconfig.rx_filter = HWTSTAMP_FILTER_NONE;

        ifreq ifr{};
        // TODO: Review
        // std::strncpy(ifr.ifr_name, kTransmitInterface.c_str(), IFNAMSIZ - 1);
        std::snprintf(ifr.ifr_name,
              sizeof(ifr.ifr_name),
              "%s",
              kTransmitInterface.c_str());
        ifr.ifr_data = reinterpret_cast<char*>(&hwconfig);

        if (ioctl(sockfd_, SIOCSHWTSTAMP, &ifr) < 0) {
            std::cerr << "ioctl SIOCSHWTSTAMP failed: " << strerror(errno) << " (Code: " << errno << ")\n";
            throw std::runtime_error("Failed to enable hardware timestamping on NIC");
        }

        // --- 3. Flag the Socket for Timestamping ---
        constexpr int ts_flags = SOF_TIMESTAMPING_TX_HARDWARE |
                                 SOF_TIMESTAMPING_RAW_HARDWARE |
                                 SOF_TIMESTAMPING_OPT_TSONLY;

        if (setsockopt(sockfd_, SOL_SOCKET, SO_TIMESTAMPING, &ts_flags, sizeof(ts_flags)) < 0) {
            std::cerr << "setsockopt SO_TIMESTAMPING failed: " << strerror(errno) << " (Code: " << errno << ")\n";
            throw std::runtime_error("Failed to set SO_TIMESTAMPING");
        }
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

    void MockUDPSender::Send([[maybe_unused]] std::span<const char> buf) {
        // no-op
    }

    void UDPSender::t(std::atomic<bool>& replay_done, std::vector<uint64_t>& timestamps) {
        msghdr msg{};
        char iov_buf[1];
        iovec msg_iov = {iov_buf, 0};
        msg.msg_iov = &msg_iov;
        msg.msg_iovlen = 1;
        // buffer has to be zero-initialised (see man 7 cmsg)
        alignas(cmsghdr) char ctrl[512]{};
        msg.msg_control = ctrl;
        msg.msg_controllen = sizeof(ctrl);

        // Polling
        pollfd fds;
        fds.fd = sockfd_;
        fds.events = POLLPRI;
        while (!replay_done.load(std::memory_order_acquire)) {
            poll(&fds, 1, 100);
            while (recvmsg(sockfd_, &msg, MSG_ERRQUEUE | MSG_DONTWAIT) >= 0) {
                for (cmsghdr* cmsg = CMSG_FIRSTHDR(&msg); cmsg != nullptr; cmsg = CMSG_NXTHDR(&msg , cmsg)) {
                    if (cmsg->cmsg_level == SOL_SOCKET && cmsg->cmsg_type == SCM_TIMESTAMPING) {
                        scm_timestamping scm_ts{};
                        memcpy(&scm_ts, CMSG_DATA(cmsg), sizeof(scm_ts));
                        uint64_t timestamp = time::timespec_to_ns(scm_ts.ts[2]);
                        timestamps.push_back(timestamp);
                    }
                }
                msg.msg_controllen = sizeof(ctrl);
                memset(&ctrl, 0, sizeof(ctrl));
            }
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                std::cerr << "recvmsg error queue failed: " << strerror(errno) << " (Code: " << errno << ")\n";
            }
        }
    }
}


