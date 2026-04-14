#include <iostream>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "itch.hpp"
#include "moldudp64.hpp"
#include "networking.hpp"

int main() {
    char buf[nanoparser::networking::kMaxUdpPayload];

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
    mreq.imr_interface.s_addr = htonl(INADDR_LOOPBACK);
    if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) == -1)
        std::cerr << "IP_ADD_MEMBERSHIP failed: " << strerror(errno) << "\n";
    else
        std::cerr << "joined multicast group 239.0.0.1\n";

    std::cerr << "waiting for packets...\n";
    for (int i = 0; i < 5; ++i) {
        ssize_t bytes = recv(sockfd, buf, sizeof(buf), 0);
        if (bytes == -1) {
            std::cerr << "recv failed: " << strerror(errno) << "\n";
            break;
        }
        std::cerr << "received packet " << i + 1 << ", bytes=" << bytes << "\n";
        std::cerr << "first byte (msg type): " << buf[nanoparser::MoldUDP64Packet::kHeaderSize + 2] << "\n";

        uint64_t timestamp = nanoparser::Itch::get_timestamp(buf + nanoparser::MoldUDP64Packet::kHeaderSize);
        std::cout << "message " << i + 1 << " timestamp: " << timestamp << "\n";
    }

    close(sockfd);
}