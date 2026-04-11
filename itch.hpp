#pragma once
#include <cstdint>


namespace nanoparser {
    class Itch {
    public:
        static constexpr uint16_t kMessageLengthSize = 2;
        static constexpr uint16_t kTimestampOffset = 5;
        static constexpr uint16_t kTimestampSize = 6;
        static constexpr uint16_t kMaxMessageSize = 50;
        static constexpr uint16_t kMaxPacketSize = kMaxMessageSize + kMessageLengthSize;

        static uint64_t get_timestamp(const char* buf);
    };
}