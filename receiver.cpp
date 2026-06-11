#include <iostream>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/errqueue.h>
#include <linux/net_tstamp.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <fstream>

#include "constants.hpp"
#include "itch.hpp"
#include "moldudp64.hpp"
#include "networking.hpp"
#include "time.hpp"

int main() {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        std::cerr << "socket failed: " << strerror(errno) << "\n";
        return 1;
    }
    std::cerr << "socket created: fd=" << sockfd << "\n";

    int reuse = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1)
        std::cerr << "SO_REUSEADDR failed: " << strerror(errno) << "\n";
    else
        std::cerr << "SO_REUSEADDR set\n";

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(12345);
    addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sockfd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1) {
        std::cerr << "bind failed: " << strerror(errno) << "\n";
        return 1;
    }
    std::cerr << "bind succeeded on port 12345\n";

    ip_mreq mreq{};
    mreq.imr_multiaddr.s_addr = inet_addr("239.0.0.1");
    mreq.imr_interface.s_addr = inet_addr(nanoparser::kReceiveIpAddress.c_str());
    if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) == -1)
        std::cerr << "IP_ADD_MEMBERSHIP failed: " << strerror(errno) << "\n";
    else
        std::cerr << "joined multicast group 239.0.0.1\n";

    // --- 2. Wake Up the NIC Hardware Clock ---
    hwtstamp_config hwconfig{};
    hwconfig.tx_type = HWTSTAMP_TX_OFF;
    hwconfig.rx_filter = HWTSTAMP_FILTER_ALL;

    ifreq ifr{};
    std::strncpy(ifr.ifr_name, nanoparser::kReceiveInterface.c_str(), IFNAMSIZ - 1);
    ifr.ifr_data = reinterpret_cast<char*>(&hwconfig);

    if (ioctl(sockfd, SIOCSHWTSTAMP, &ifr) < 0) {
        std::cerr << "ioctl SIOCSHWTSTAMP failed: " << strerror(errno) << " (Code: " << errno << ")\n";
        throw std::runtime_error("Failed to enable hardware timestamping on NIC");
    }
    std::cerr << "actual rx_filter: " << hwconfig.rx_filter << "\n";

    // --- 3. Flag the Socket for Timestamping ---
    constexpr int ts_flags = SOF_TIMESTAMPING_RX_HARDWARE |
                         SOF_TIMESTAMPING_RAW_HARDWARE;

    if (setsockopt(sockfd, SOL_SOCKET, SO_TIMESTAMPING, &ts_flags, sizeof(ts_flags)) < 0) {
        std::cerr << "setsockopt SO_TIMESTAMPING failed: " << strerror(errno) << " (Code: " << errno << ")\n";
        throw std::runtime_error("Failed to set SO_TIMESTAMPING");
    }

    timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    // Setting the timeout on a blocking socket
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv) < 0) {
        std::cerr << "setsockopt SO_RCVTIMEO failed: " << strerror(errno) << " (Code: " << errno << ")\n";
        throw std::runtime_error("Failed to set receive socket timeout");
    }

    std::cerr << "waiting for packets...\n";


    // ------------------------------------------------------------------------------------------
    // Extract timestamps
    // ------------------------------------------------------------------------------------------
    msghdr msg{};
    char iov_buf[nanoparser::MoldUDP64Packet::kMaxSize];
    iovec msg_iov = {iov_buf, nanoparser::MoldUDP64Packet::kMaxSize};
    msg.msg_iov = &msg_iov;
    msg.msg_iovlen = 1;
    // buffer has to be zero-initialised (see man 7 cmsg)
    alignas(cmsghdr) char ctrl[512]{};
    msg.msg_control = ctrl;
    msg.msg_controllen = sizeof(ctrl);
    std::vector<uint64_t> timestamps;

    while (true) {
        msg.msg_controllen = sizeof(ctrl);
        memset(&ctrl, 0, sizeof(ctrl));
        auto bytes = recvmsg(sockfd, &msg, 0);
        if (bytes < 0) {
            if ((errno == EAGAIN || errno == EWOULDBLOCK)) {
                if (timestamps.empty())
                    continue;
                break;
            }
            std::cerr << "recvmsg of RX timestamps failed. " << strerror(errno) << " (Code: " << errno << ")\n";
        }
        for (cmsghdr* cmsg = CMSG_FIRSTHDR(&msg); cmsg != nullptr; cmsg = CMSG_NXTHDR(&msg , cmsg)) {
            if (cmsg->cmsg_level == SOL_SOCKET && cmsg->cmsg_type == SCM_TIMESTAMPING) {
                scm_timestamping scm_ts{};
                memcpy(&scm_ts, CMSG_DATA(cmsg), sizeof(scm_ts));
                uint64_t timestamp = nanoparser::time::timespec_to_ns(scm_ts.ts[2]);
                timestamps.push_back(timestamp);
            }
        }
    }

    close(sockfd);
    std::ofstream f("/home/illia-derevianko/Projects/nanoparser/analysis/rx_times", std::ios::binary);
    f.write(reinterpret_cast<const char*>(timestamps.data()),
            static_cast<std::streamsize>(timestamps.size() * sizeof(uint64_t)));
    std::cout << "Total number of timestamps: " << timestamps.size() << std::endl;
}
