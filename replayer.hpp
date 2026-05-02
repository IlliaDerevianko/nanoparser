#pragma once

#include <limits>
#include <string>
#include <vector>

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
            , udp_sender_(config.ip, config.port)
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
            // TRACING
            send_times_.reserve(20'000);
            ideal_times_.reserve(20'000);
        }

        void replay(uint64_t start = 0, uint64_t end = std::numeric_limits<uint64_t>::max()) {
            consume_first_msg_after(start);
            reader_.prefetch();
            start_time_ = time::get_time_ns();
            last_sent_time_ = start_time_;

            uint64_t last_msg_timestamp = first_timestamp_;

            std::span<const char> msg{};
            while (!(msg = reader_.read()).empty()) {
                uint64_t timestamp = Itch::get_timestamp(msg.data());
                if (timestamp > end)
                    break;
                // send as soon as the MoldUDP64 packet is full (current message doesn't fit)
                if (mold_udp64_packet_.bytes_remaining() < msg.size() && !mold_udp64_packet_.empty()) {
                    send();
                    // TRACING
                    ideal_times_.push_back(last_msg_timestamp - first_timestamp_ + start_time_);
                }
                uint64_t now{};
                // spin until this ITCH message has to be sent
                while ((now = time::get_time_ns()) - start_time_ < timestamp - first_timestamp_) {
                    // send if the maximum inter-packet interval is exceeded
                    if (now - last_sent_time_ > max_inter_packet_interval_ && !mold_udp64_packet_.empty()) {
                        send();
                        // TRACING
                        ideal_times_.push_back(last_msg_timestamp - first_timestamp_ + start_time_);
                    }
                }

                mold_udp64_packet_.append_msg({msg.data(), msg.size()});
                last_msg_timestamp = timestamp;
            }
            // send the leftover ITCH messages
            if (!mold_udp64_packet_.empty()) {
                send();
                // TRACING
                ideal_times_.push_back(last_msg_timestamp - first_timestamp_ + start_time_);
            }

            dump_send_times();
        }

    private:
        Reader reader_;
        UDPSender udp_sender_;
        uint64_t max_inter_packet_interval_;
        uint64_t first_timestamp_;
        uint64_t start_time_;
        uint64_t last_sent_time_;
        uint64_t seq_num_;
        char packet_buf_[MoldUDP64Packet::kMaxSize];
        MoldUDP64Packet mold_udp64_packet_;
        // TRACING
        std::vector<uint64_t> send_times_;
        std::vector<uint64_t> ideal_times_;

        void consume_first_msg_after(uint64_t start) {
            std::span<const char> msg{};

            while (!(msg = reader_.read()).empty()) {
                if (uint64_t timestamp = Itch::get_timestamp(msg.data()); timestamp >= start) {
                    first_timestamp_ = timestamp;
                    mold_udp64_packet_.append_msg({msg.data(), msg.size()});
                    return;
                }
            }
        }

        void send() {
            mold_udp64_packet_.set_seq_num(seq_num_);
            auto sent = mold_udp64_packet_.send(udp_sender_);
            seq_num_ += sent;
            last_sent_time_ = time::get_time_ns();
            // TRACING
            send_times_.push_back(last_sent_time_);
        }

        void dump_send_times() const {
            std::ofstream f1("/home/illiaderevianko/Desktop/projects/nanoparser/data/send_times",
                std::ios::binary);
            std::ofstream f2("/home/illiaderevianko/Desktop/projects/nanoparser/data/ideal_times",
                std::ios::binary);
            f1.write(reinterpret_cast<const char*>(send_times_.data()),
                    static_cast<std::streamsize>(send_times_.size() * sizeof(uint64_t)));
            f2.write(reinterpret_cast<const char*>(ideal_times_.data()),
                    static_cast<std::streamsize>(ideal_times_.size() * sizeof(uint64_t)));
        }
    };
}
