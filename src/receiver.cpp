#include "receiver.hpp"

#include <fstream>


#include "constants.hpp"
#include "networking/cmsg_handlers.hpp"
#include "networking/rx_udp_socket.hpp"

namespace nanoparser {
    void Receiver::Receive(const std::chrono::milliseconds timeout) {
        // Configure the receiving socket.
        RxUdpSocket socket(kReceiveIpAddress, "239.0.0.1", 12345);
        socket.EnableRxHwTimestamping(kReceiveInterface);
        socket.SetReceiveTimeout(timeout);

        MessageHeader msg_header;
        // Loop continues as long as message inter-arrival time stays within the timeout.
        while (true) {
            msg_header.ResetCtrlBuf();
            if (auto recv_res = socket.ReceiveMsg(msg_header); !recv_res.has_value()) {
                if (recv_res.error() == RecvStatus::NoDataAvailable) {
                    // Loop keeps spinning if no messages have been received yet.
                    if (timestamps_.empty()) continue;
                    break;
                }
                throw std::runtime_error("failed to receive a message");
            }

            TimestampExtractor ts_extractor{};
            msg_header.ForEachCmsg(ts_extractor);
            timestamps_.push_back(ts_extractor.timestamp);
        }
    }

    void Receiver::DumpTimestamps() const {
        std::ofstream f("/home/illia-derevianko/Projects/nanoparser/analysis/rx_times", std::ios::binary);
        f.write(reinterpret_cast<const char*>(timestamps_.data()),
                static_cast<std::streamsize>(timestamps_.size() * sizeof(uint64_t)));
        std::cout << "Total number of timestamps: " << timestamps_.size() << std::endl;
    }
}
