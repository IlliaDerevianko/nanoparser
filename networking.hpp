#pragma once
#include <span>
#include <string>
#include <netinet/in.h>

namespace nanoparser::networking{
    constexpr size_t kMaxUdpPayload = 1472;
}

namespace nanoparser {
    class UDPSender {
    public:
        UDPSender(std::string_view multicast_ip, uint16_t dst_port, std::string_view src_ip);
        UDPSender(UDPSender&&) noexcept ;
        UDPSender& operator=(UDPSender&&) noexcept;
        UDPSender(const UDPSender&) = delete;
        UDPSender& operator=(const UDPSender&) = delete;
        ~UDPSender();
        ssize_t send(std::span<const char> buf);

    private:
        int sockfd_;
        sockaddr_in addr_;
    };


    class MockUDPSender {
    public:
        MockUDPSender() = default;
        MockUDPSender(const MockUDPSender&) = delete;
        MockUDPSender& operator=(const MockUDPSender&) = delete;

        void send(std::span<const char> buf);
    };
}

