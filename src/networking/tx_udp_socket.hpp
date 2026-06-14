#pragma once

#include <cstdint>
#include <span>
#include <string>

#include "udp_socket.hpp"

namespace nanoparser {
    class TxUdpSocket : UdpSocket {
    public:
        TxUdpSocket(const std::string& src_ip, const std::string& multicast_group_ip, uint16_t dst_port);
        void Send(std::span<const char> buf) const;
        void EnableTxHwTimestamping(const std::string& src_interface) const;
    };
}


