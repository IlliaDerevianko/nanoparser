#pragma once
#include <chrono>

#include "networking/rx_udp_socket.hpp"


namespace nanoparser {
    struct ReceiverConfig {
        std::chrono::milliseconds timeout;
        std::string multicast_ip;
        std::string dst_ip;
        std::uint16_t dst_port;
        std::string rx_interface;
    };

    class Receiver {
    public:
        explicit Receiver(const ReceiverConfig& config);
        void Receive();
        void DumpTimestamps() const;
    private:
        static constexpr int kExpectedTimestampCount = 20'000;
        RxUdpSocket socket_;
        std::vector<uint64_t> rx_hw_timestamps_;
        std::vector<uint64_t> receive_app_timestamps_;
    };
}
