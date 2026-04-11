#include "replayer.hpp"
#include "itch.hpp"
#include "moldudp64.hpp"
#include "time.hpp"

namespace nanoparser {
    Replayer::Replayer(std::string_view data_src_path, const ReplayerConfig& config)
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

    void Replayer::replay(uint64_t start, uint64_t end) {
        char msg_buf[Itch::kMaxPacketSize];

        consume_first_msg_after(start);

        while (uint16_t msg_size = reader_.read(msg_buf)) {
            uint64_t timestamp = Itch::get_timestamp(msg_buf);
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
            if (mold_udp64_packet_.bytes_remaining() < msg_size && !mold_udp64_packet_.empty()) {
                send();
            }

            mold_udp64_packet_.append_msg({msg_buf, msg_size});
        }
        // send the leftover ITCH messages
        if (!mold_udp64_packet_.empty())
            send();
    }

    void Replayer::consume_first_msg_after(uint64_t start) {
        char msg_buf[Itch::kMaxPacketSize];

        while (uint16_t msg_size = reader_.read(msg_buf)) {
            if (uint64_t timestamp = Itch::get_timestamp(msg_buf); timestamp >= start) {
                first_timestamp_ = timestamp;
                start_time_ = time::get_time_ns();
                last_sent_time_ = start_time_;
                mold_udp64_packet_.append_msg({msg_buf, msg_size});
                return;
            }
        }
    }

    void Replayer::send() {
        mold_udp64_packet_.set_seq_num(seq_num_);
        auto sent = mold_udp64_packet_.send(socket_);
        seq_num_ += sent;
        last_sent_time_ = time::get_time_ns();
    }
}
