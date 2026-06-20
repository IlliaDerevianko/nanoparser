#pragma once
#include <chrono>
#include <expected>
#include <string>

#include "message_header.hpp"
#include "udp_socket.hpp"

namespace nanoparser {

    class RxUdpSocket : public UdpSocket {
    public:
        RxUdpSocket(const std::string& dst_ip, const std::string& multicast_group_ip, uint16_t port);
        void EnableRxHwTimestamping(const std::string& dst_interface) const;
        void SetReceiveTimeout(std::chrono::milliseconds timeout) const;

        template<size_t DataBufSize, size_t CtrlBufSize>
        std::expected<ssize_t, RecvStatus> ReceiveMsg(MessageHeader<DataBufSize, CtrlBufSize>& msg_header) {
            return Receive(msg_header, 0);
        }
    };
}

