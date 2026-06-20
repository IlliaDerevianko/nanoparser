#pragma once

#include <iostream>
#include <limits>
#include <string>
#include <vector>
#include <thread>

#include "moldudp64.hpp"
#include "networking/null_tx_udp_socket.hpp"
#include "reader_stream.hpp"
#include "reader_mmap.hpp"
#include "time.hpp"
#include "networking/cmsg_handlers.hpp"
#include "networking/message_header.hpp"
#include "networking/poller.hpp"
#include "networking/rx_udp_socket.hpp"
#include "networking/tx_udp_socket.hpp"

namespace nanoparser {
    struct ReplayerConfig {
        uint64_t max_msg_delay;
        std::string multicast_ip;
        std::string src_ip;
        std::uint16_t dst_port;
        std::string tx_interface;
    };

    template<typename Reader>
    class Replayer {
    public:
        explicit Replayer(const std::filesystem::path& data_src_path, const ReplayerConfig& config)
            : reader_(data_src_path)
            , socket_(config.src_ip, config.multicast_ip, config.dst_port)
            , max_msg_delay_(config.max_msg_delay)
            , first_msg_timestamp_{}
            , start_time_{}
            , seq_num_(0)
            , first_msg_checkpoint_(reader_.get_begin_checkpoint())
            , is_replay_completed_(false)
        {
            constexpr std::array kSession = {'0', '1', '3', '0', '2', '0', '1', '9', ' ', ' '};
            mold_udp64_packet_.set_session(kSession);
            socket_.EnableTxHwTimestamping(config.tx_interface);
            // TRACING
            send_times_.reserve(kExpectedTimestampCount);
            ideal_times_.reserve(kExpectedTimestampCount);
        }

        void replay(uint64_t start = 0, uint64_t end = std::numeric_limits<uint64_t>::max()) {
            seek_to_first_from(start);
            record_ideal_times(end);
            reader_.prefetch();

            ReadResult<Reader> read_res;
            uint64_t first_msg_in_packet_time{}; // the value is only meaningful if the packet is non-empty
            std::thread ts_consumer_thread = launch_timestamp_gatherer();

            // cpu_set_t set;
            // CPU_ZERO(&set);
            // CPU_SET(5, &set);
            // if (sched_setaffinity(0, sizeof(set), &set) == -1) {
            //     std::cerr << "Failed to set affinity" << std::endl;
            // }
            //
            // sched_param param = { .sched_priority = 99 };
            // if (sched_setscheduler(0, SCHED_FIFO, &param) == -1) {
            //     std::cerr << "Failed to set priority" << std::endl;
            // }
            // Start of the replay
            start_time_ = time::get_time_ns();
            while (!(read_res = reader_.read()).empty()) {
                auto [msg, _] = read_res;
                uint64_t timestamp = Itch::get_timestamp(msg.data());
                // force the replayer to stop early if the message's timestamp is after the end
                auto target_timestamp = std::max(std::min(timestamp, end), first_msg_timestamp_);
                uint64_t now{};
                // spin until this message is ready to be sent
                while ((now = time::get_time_ns()) - start_time_ < target_timestamp - first_msg_timestamp_) {
                    // send the packet if the maximum message delay time (due to coalescing) is exceeded
                    if (!mold_udp64_packet_.empty() && now - first_msg_in_packet_time > max_msg_delay_) {
                        send();
                    }
                }
                // although this exit condition could be placed before the busy waiting loop, placing it here
                // ensures that the factual replay time is closer to (end - start)
                if (timestamp > end)
                    break;
                // send the packet if the current message causes an overflow
                if (!mold_udp64_packet_.empty() && mold_udp64_packet_.bytes_remaining() < msg.size()) {
                    send();
                }

                if (mold_udp64_packet_.empty())
                    first_msg_in_packet_time = time::get_time_ns();
                mold_udp64_packet_.append_msg(msg);
            }
            // send the leftover packet
            if (!mold_udp64_packet_.empty()) {
                send();
            }
            is_replay_completed_.store(true, std::memory_order_release);
            ts_consumer_thread.join();
            dump_timestamps();
        }

    private:
        static constexpr int kExpectedTimestampCount = 20'000;

        Reader reader_;
        TxUdpSocket socket_;
        uint64_t max_msg_delay_;
        uint64_t first_msg_timestamp_;
        uint64_t start_time_;
        uint64_t seq_num_;
        MoldUDP64Packet mold_udp64_packet_;
        ReaderCheckpoint<Reader> first_msg_checkpoint_;
        std::atomic<bool> is_replay_completed_;
        // TRACING
        std::vector<uint64_t> send_times_;
        std::vector<uint64_t> ideal_times_;

        void seek_to_first_from(uint64_t start) {
            ReadResult<Reader> read_res;

            while (!(read_res = reader_.read()).empty()) {
                auto [msg, checkpoint] = read_res;
                if (uint64_t timestamp = Itch::get_timestamp(msg.data()); timestamp >= start) {
                    first_msg_timestamp_ = timestamp;
                    return;
                }
                first_msg_checkpoint_ = checkpoint;
                reader_.seek(first_msg_checkpoint_);
            }
        }

        void send() {
            mold_udp64_packet_.set_seq_num(seq_num_);
            auto sent = mold_udp64_packet_.Send(socket_);
            seq_num_ += sent;
            // TRACING
            //send_times_.push_back(time::get_time_ns() - start_time_);
        }

        void record_ideal_times(uint64_t end) {
            auto initial_checkpoint = reader_.get_current_checkpoint();

            MoldUDP64Packet mock_packet;
            NullTxUdpSocket mock_sender;
            ReadResult<Reader> read_res;
            uint64_t first_msg_in_packet_timestamp{};
            uint64_t prev_msg_timestamp{}; // tracked solely for the EOF case

            while (!(read_res = reader_.read()).empty()) {
                auto [msg, _] = read_res;
                uint64_t timestamp = Itch::get_timestamp(msg.data());

                if (timestamp > end) {
                    if (!mock_packet.empty())
                        ideal_times_.push_back(std::min(first_msg_in_packet_timestamp + max_msg_delay_, end) -
                            first_msg_timestamp_);
                    reader_.seek(initial_checkpoint);
                    return;
                }

                // TIMEOUT case:
                if (!mock_packet.empty() && timestamp - first_msg_in_packet_timestamp > max_msg_delay_ ) {
                    ideal_times_.push_back(first_msg_in_packet_timestamp + max_msg_delay_ - first_msg_timestamp_);
                    mock_packet.Send(mock_sender);
                // MTU case:
                } else if (!mock_packet.empty() && msg.size() > mock_packet.bytes_remaining()) {
                    ideal_times_.push_back(timestamp - first_msg_timestamp_);
                    mock_packet.Send(mock_sender);
                }

                if (mock_packet.empty())
                    first_msg_in_packet_timestamp = timestamp;
                mock_packet.append_msg(msg);
                prev_msg_timestamp = timestamp;
            }

            // EOF case that can only be reached if end >= last timestamp
            if (!mock_packet.empty()) {
                ideal_times_.push_back(prev_msg_timestamp - first_msg_timestamp_);
            }

            reader_.seek(initial_checkpoint);
        }

        std::thread launch_timestamp_gatherer() {
            using namespace std::chrono_literals;
            std::thread t([this] () {
                MessageHeaderNoPayload msg_header;
                Poller poller(socket_);
                bool is_last_pass = false;
                while (true) {
                    poller.Poll(100ms);
                    while (true) {
                        auto result = socket_.ReceiveErrQueueMsg(msg_header);
                        if (!result.has_value()) {
                            if (result.error() == RecvStatus::UnknownFatal) {
                                throw std::runtime_error("recvmsg failed");
                            }
                            break;
                        }
                        TimestampExtractor ts_extractor{};
                        msg_header.ForEachCmsg(ts_extractor);
                        send_times_.push_back(ts_extractor.timestamp);
                    }
                    if (is_last_pass)
                        break;
                    if (is_replay_completed_.load(std::memory_order_acquire)) {
                        is_last_pass = true;
                    }
                }
            });
            return t;
        }

        void dump_timestamps() const {
            std::ofstream f1("/home/illia-derevianko/Projects/nanoparser/analysis/send_times",
                std::ios::binary);
            f1.write(reinterpret_cast<const char*>(send_times_.data()),
                    static_cast<std::streamsize>(send_times_.size() * sizeof(uint64_t)));
            std::ofstream f2("/home/illia-derevianko/Projects/nanoparser/analysis/ideal_times",
                std::ios::binary);
            f2.write(reinterpret_cast<const char*>(ideal_times_.data()),
                    static_cast<std::streamsize>(ideal_times_.size() * sizeof(uint64_t)));

            std::cout << send_times_.size();
            if (!send_times_.empty())
                std::cout << " " << (send_times_.back() - send_times_.front());
            std::cout << std::endl;

            std::cout << ideal_times_.size();
            if (!ideal_times_.empty())
                std::cout << " " << (ideal_times_.back() - ideal_times_.front());
            std::cout << std::endl;
        }
    };
}
