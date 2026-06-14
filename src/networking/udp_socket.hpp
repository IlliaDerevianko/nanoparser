#pragma once
#include <expected>
#include <iostream>

#include "message_header.hpp"

namespace nanoparser {
    enum class RecvStatus {
        NoDataAvailable,
        UnknownFatal
    };

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

        template <size_t DataBufSize, size_t CtrlBufSize>
        std::expected<ssize_t, RecvStatus>
        Receive(MessageHeader<DataBufSize, CtrlBufSize>& msg_header, int flags) {
            msg_header.ResetCtrlBuf();
            ssize_t bytes = recvmsg(sockfd_, msg_header.data(), flags);
            if (bytes == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    return std::unexpected(RecvStatus::NoDataAvailable);
                }
                std::cerr << "recvmsg failed: " << strerror(errno) << " (Code: " << errno << ")";
                return std::unexpected(RecvStatus::UnknownFatal);
            }
            return bytes;
        }
    };
}

