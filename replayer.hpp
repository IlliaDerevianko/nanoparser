#pragma once

#include <limits>
#include <string>

#include "moldudp64.hpp"
#include "networking.hpp"
#include "reader_stream.hpp"
#include "reader_mmap.hpp"
#include "time.hpp"

namespace nanoparser {
    struct ReplayerConfig {
        uint64_t max_inter_packet_interval = 10'000'000;
        std::string ip = "239.0.0.1";
        std::uint16_t port = 12345;
    };

    template<typename Reader>
    class Replayer {
    public:
        explicit Replayer(const std::string& data_src_path, const ReplayerConfig& config = ReplayerConfig{})
            : reader_(data_src_path)
            , socket_(config.ip, config.port)
            , max_inter_packet_interval_(config.max_inter_packet_interval)
            , first_timestamp_{}
            , start_time_{}
            , last_sent_time_{}
            , seq_num_(0)
            , packet_buf_{}
            , mold_udp64_packet_(packet_buf_)
        {
            constexpr std::array kSession = {'0', '1', '3', '0', '2', '0', '1', '9', ' ', ' '};
            mold_udp64_packet_.set_session(kSession);
        }

        void replay(uint64_t start = 0, uint64_t end = std::numeric_limits<uint64_t>::max()) {
            consume_first_msg_after(start);

            std::span<const char> msg{};
            while (!(msg = reader_.read()).empty()) {
                uint64_t timestamp = Itch::get_timestamp(msg.data());
                if (timestamp > end)
                    break;
                uint64_t now{};
                // spin until this ITCH message has to be sent
                while ((now = time::get_time_ns()) - start_time_ < timestamp - first_timestamp_) {
                    // send if the maximum inter-packet interval is exceeded
                    if (now - last_sent_time_ > max_inter_packet_interval_ && !mold_udp64_packet_.empty()) {
                        send();
                    }
                }
                // send as soon as the MoldUDP64 packet is full
                if (mold_udp64_packet_.bytes_remaining() < msg.size() && !mold_udp64_packet_.empty()) {
                    send();
                }

                mold_udp64_packet_.append_msg({msg.data(), msg.size()});
            }
            // send the leftover ITCH messages
            if (!mold_udp64_packet_.empty())
                send();
        }

    private:
        Reader reader_;
        UDPsocket socket_;
        uint64_t max_inter_packet_interval_;
        uint64_t first_timestamp_;
        uint64_t start_time_;
        uint64_t last_sent_time_;
        uint64_t seq_num_;
        char packet_buf_[MoldUDP64Packet::kMaxSize];
        MoldUDP64Packet mold_udp64_packet_;

        void consume_first_msg_after(uint64_t start) {
            std::span<const char> msg{};

            while (!(msg = reader_.read()).empty()) {
                if (uint64_t timestamp = Itch::get_timestamp(msg.data()); timestamp >= start) {
                    first_timestamp_ = timestamp;
                    start_time_ = time::get_time_ns();
                    last_sent_time_ = start_time_;
                    mold_udp64_packet_.append_msg({msg.data(), msg.size()});
                    return;
                }
            }
        }

        void send() {
            mold_udp64_packet_.set_seq_num(seq_num_);
            auto sent = mold_udp64_packet_.send(socket_);
            seq_num_ += sent;
            last_sent_time_ = time::get_time_ns();
        }
    };
}
