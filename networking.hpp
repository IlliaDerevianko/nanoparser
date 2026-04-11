#pragma once
#include <span>
#include <string>
#include <netinet/in.h>

namespace nanoparser::networking{
    constexpr size_t kMaxUdpPayload = 1472;
}

namespace nanoparser {
    class UDPsocket {
    public:
        UDPsocket(std::string_view ip_addr, uint16_t port);
        UDPsocket(UDPsocket&&) noexcept ;
        UDPsocket& operator=(UDPsocket&&) noexcept;
        UDPsocket(const UDPsocket&) = delete;
        UDPsocket& operator=(const UDPsocket&) = delete;
        ~UDPsocket();
        ssize_t send(std::span<const char> buf);

    private:
        int sockfd_;
        sockaddr_in addr_;
    };
}

