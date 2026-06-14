#pragma once
#include <array>
#include <cstring>
#include <span>

#include "networking.hpp"


namespace nanoparser {
    class MoldUDP64Packet {
    public:
        static constexpr uint16_t kSessionSize = 10;
        static constexpr uint16_t kSequenceNumberSize = 8;
        static constexpr uint16_t kMessageCountSize = 2;
        static constexpr uint16_t kHeaderSize = kSessionSize + kSequenceNumberSize + kMessageCountSize;
        static constexpr uint16_t kMaxSize = networking::kMaxUdpPayload;
        static constexpr uint16_t kMaxPayload = kMaxSize - kHeaderSize;

        MoldUDP64Packet();

        void set_session(std::span<const char, 10> id);

        void set_seq_num(uint64_t seq_num);

        void append_msg(std::span<const char> msg_buf);

        [[nodiscard]] size_t bytes_remaining() const;

        [[nodiscard]] bool empty() const;

        template<typename Sender>
        uint16_t Send(Sender& sender) {
            const uint16_t msg_cnt_be = std::byteswap(msg_cnt_);
            memcpy(buf_.data() + kSessionSize + kSequenceNumberSize, &msg_cnt_be, sizeof(msg_cnt_be));
            sender.Send({buf_.data(), size_});
            auto sent = msg_cnt_;
            msg_cnt_ = 0;
            size_ = kHeaderSize;
            return sent;
        }

    private:
        std::array<char, kMaxSize> buf_;
        uint16_t msg_cnt_;
        uint16_t size_;
    };
}
