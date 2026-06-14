#pragma once

namespace nanoparser {
    class UdpSocket {
    public:
        UdpSocket();
        UdpSocket(const UdpSocket&) = delete;
        UdpSocket& operator=(const UdpSocket&) = delete;
        UdpSocket(UdpSocket&&) noexcept;
        UdpSocket& operator=(UdpSocket&&) noexcept;
        ~UdpSocket();

        [[nodiscard]] int Fd() const noexcept;

    protected:
        int sockfd_;
    };
}

