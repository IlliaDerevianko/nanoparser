#include "receiver.hpp"

#include <fstream>
#include <ranges>

#include "dotenv.h"
#include "time.hpp"
#include "networking/cmsg_handlers.hpp"
#include "networking/rx_udp_socket.hpp"

namespace nanoparser {
    Receiver::Receiver(const ReceiverConfig& config)
        : socket_(config.dst_ip, config.multicast_ip, config.dst_port)
    {
        socket_.EnableRxHwTimestamping(config.rx_interface);
        socket_.SetReceiveTimeout(config.timeout);
        timestamps_.reserve(kExpectedTimestampCount);
    }

    void Receiver::Receive() {
        MessageHeader msg_header;
        // Loop continues as long as message inter-arrival time stays within the timeout.
        while (true) {
            msg_header.ResetCtrlBuf();
            if (auto recv_res = socket_.ReceiveMsg(msg_header); recv_res.has_value()) {
                const uint64_t receive_timestamp = time::get_time_ns();
                TimestampExtractor ts_extractor{};
                msg_header.ForEachCmsg(ts_extractor);
                timestamps_.push_back({ts_extractor.timestamp, receive_timestamp});
            } else {
                if (recv_res.error() == RecvStatus::NoDataAvailable) {
                    // Loop keeps spinning if no messages have been received yet.
                    if (timestamps_.empty()) continue;
                    break;
                }
                throw std::runtime_error("failed to receive a message");
            }
        }
    }

    void Receiver::DumpTimestamps() const {
        std::string path1 = dotenv::getenv("RX_TIMES_PATH") + dotenv::getenv("EXP");
        std::string path2 = dotenv::getenv("RECEIVE_APP_TIMES_PATH") + dotenv::getenv("EXP");
        std::ofstream f1(path1, std::ios::binary);
        std::ofstream f2(path2, std::ios::binary);

        auto rx_hw_timestamps = timestamps_
            | std::views::transform([](const auto& t) { return t[0]; })
            | std::ranges::to<std::vector>();
        auto receive_app_timestamps = timestamps_
            | std::views::transform([](const auto& t) { return t[1]; })
            | std::ranges::to<std::vector>();
        f1.write(reinterpret_cast<const char*>(rx_hw_timestamps.data()),
                static_cast<std::streamsize>(rx_hw_timestamps.size() * sizeof(uint64_t)));

        f2.write(reinterpret_cast<const char*>(receive_app_timestamps.data()),
                static_cast<std::streamsize>(receive_app_timestamps.size() * sizeof(uint64_t)));
        std::cout << "Total number timestamps: " << timestamps_.size() << std::endl;
    }
}
