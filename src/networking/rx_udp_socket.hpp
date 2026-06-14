#pragma once
#include <chrono>
#include <expected>
#include <iostream>
#include <string>

#include "message_header.hpp"
#include "udp_socket.hpp"

namespace nanoparser {
    enum class RecvStatus {
        NoDataAvailable,
        UnknownFatal
    };

    class RxUdpSocket : public UdpSocket {
    public:
        RxUdpSocket(const std::string& dst_ip, const std::string& multicast_group_ip, uint16_t port);
        void EnableRxHwTimestamping(const std::string& dst_interface) const;
        void SetReceiveTimeout(std::chrono::milliseconds timeout) const;

        template<size_t DataBufSize, size_t CtrlBufSize>
        std::expected<ssize_t, RecvStatus> Receive(MessageHeader<DataBufSize, CtrlBufSize>& msg_header) {
            msg_header.ResetCtrlBuf();
            ssize_t bytes = recvmsg(sockfd_, msg_header.data(), 0);
            if (bytes == -1) {
                if ((errno == EAGAIN || errno == EWOULDBLOCK)) {
                    return std::unexpected(RecvStatus::NoDataAvailable);
                }
                std::cerr << "recvmsg failed: " << strerror(errno) << " (Code: " << errno << ")";
                return std::unexpected(RecvStatus::UnknownFatal);
            }
            return bytes;
        }
    };
}

