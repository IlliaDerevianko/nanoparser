#pragma once
#include "networking.hpp"

namespace moldudp64 {
    constexpr size_t kSessionSize = 10;
    constexpr size_t kSequenceNumberSize = 8;
    constexpr size_t kMessageCountSize = 2;
    constexpr size_t kMessageLengthSize = 2;
    constexpr size_t kHeaderSize = kSessionSize + kSequenceNumberSize + kMessageCountSize;
    constexpr size_t kMaxPayload =
        networking::kMaxUdpPayload - kHeaderSize;

    using MessageLength = uint16_t;
}
