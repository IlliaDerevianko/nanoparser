#pragma once
#include <span>


namespace nanoparser {
    class NullTxUdpSocket {
    public:
        NullTxUdpSocket() = default;
        NullTxUdpSocket(const NullTxUdpSocket&) = delete;
        NullTxUdpSocket& operator=(const NullTxUdpSocket&) = delete;

        void Send(std::span<const char> buf);
    };
}

