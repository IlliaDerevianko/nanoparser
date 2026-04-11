#include "itch.hpp"

#include <bit>
#include <climits>
#include <cstring>

namespace nanoparser {
    uint64_t Itch::get_timestamp(const char *buf) {
        uint64_t timestamp_be = 0;
        memcpy(&timestamp_be, buf + kMessageLengthSize + kTimestampOffset, kTimestampSize);
        return std::byteswap(timestamp_be) >> ((sizeof(timestamp_be) - kTimestampSize) * CHAR_BIT);
    }
}
