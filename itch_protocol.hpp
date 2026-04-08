#pragma once
#include <cstdint>

namespace itch {
    constexpr int kMessageSizeFieldSize = 2;
    constexpr int kTimestampOffset = 5;
    constexpr int kTimestampSize = 6;
    constexpr int kMaxMessageSize = 50;

    using Timestamp = uint64_t;
}
