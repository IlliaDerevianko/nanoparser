#pragma once
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

        MoldUDP64Packet(char* buf);

        void set_session(std::span<const char, 10> id) const;

        void set_seq_num(uint64_t seq_num) const;

        void append_msg(std::span<const char> msg_buf);

        [[nodiscard]] size_t bytes_remaining() const;

        [[nodiscard]] bool empty() const;

        uint16_t send(UDPSender& udp_sender);

    private:
        char* buf_;
        uint16_t msg_cnt_;
        uint16_t size_;
    };
}
