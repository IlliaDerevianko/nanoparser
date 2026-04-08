#include <bit>
#include <fstream>
#include <iosfwd>
#include <iostream>
#include <string>

#include "itch_protocol.hpp"
#include "moldudp64_protocol.hpp"

const std::string kHistoricalDataPath =
    "/Users/illiaderevianko/Desktop/c++/nanoparser/data/01302019.NASDAQ_ITCH50";
const std::string kHistoricalDataTimestampsPath =
    "/Users/illiaderevianko/Desktop/c++/nanoparser/data/01302019.NASDAQ_ITCH50_timestamps";
constexpr int kBufferSize = 8192;

static itch::Timestamp write_buf[kBufferSize];

int main() {
    std::ifstream ifstrm(kHistoricalDataPath, std::ios::binary);
    std::ofstream ofstrm(kHistoricalDataTimestampsPath, std::ios::binary);
    int cnt = 0;
    // fields of interest
    moldudp64::MessageLength size;
    itch::Timestamp timestamp;
    while (ifstrm.read(reinterpret_cast<char *>(&size), moldudp64::kMessageLengthSize)) {
        size = std::byteswap(size);
        // extract the timestamp
        ifstrm.seekg(itch::kTimestampOffset, std::ios::cur);
        ifstrm.read(reinterpret_cast<char *>(&timestamp), itch::kTimestampSize);
        timestamp = std::byteswap(timestamp) >> ((sizeof(timestamp) - itch::kTimestampSize) * CHAR_BIT);
        // seek to the beginning of the next message
        ifstrm.seekg(size - itch::kTimestampOffset - itch::kTimestampSize, std::ios::cur);
        // write to buffer
        write_buf[cnt % kBufferSize] = timestamp;

        ++cnt;
        // flush if needed
        if (cnt % kBufferSize == 0) {
            ofstrm.write(reinterpret_cast<char*>(&write_buf), sizeof(timestamp) * kBufferSize);
        }

        if (cnt % 10'000'000 == 0)
            std::cout << "Messages processed: " << cnt << std::endl;
    }
    // flush the remaining timestamps
    if (int remaining = cnt % kBufferSize; remaining > 0) {
        ofstrm.write(reinterpret_cast<char*>(write_buf), static_cast<int>(sizeof(uint64_t)) * remaining);
    }
    std::cout << "Total count is: " << cnt << std::endl;
}
