#include <bit>
#include <fstream>
#include <iostream>
#include <string>
#include <cstring>
#include <ctime>

#include "itch_protocol.hpp"
#include "moldudp64_protocol.hpp"
#include "networking.hpp"

const std::string kHistoricalDataPath =
    "/Users/illiaderevianko/Desktop/c++/nanoparser/data/01302019.NASDAQ_ITCH50";

constexpr uint64_t kMaxPacketInterval = 10'000'000;
constexpr char kSession[moldudp64::kSessionSize] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

uint64_t get_time_ns() {
    std::timespec ts{};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return 1'000'000'000ULL * ts.tv_sec + ts.tv_nsec;
}

uint64_t hh_mm_to_ns_after_midnight(const unsigned hh, const unsigned mm) {
    return (hh * 60 + mm) * 60 * 1'000'000'000ULL;
}

void replay(const std::string& filepath, const uint64_t period_start, const uint64_t period_end) {
    std::ifstream ifstrm(filepath, std::ios::binary);
    char msg_block_buf[moldudp64::kMessageLengthSize + itch::kMaxMessageSize];

    // initialise moldudp64 packet
    char packet_buf[moldudp64::kMaxPayload];
    memcpy(packet_buf, kSession, moldudp64::kSessionSize);
    size_t packet_len = moldudp64::kHeaderSize;
    uint16_t msg_cnt = 0;

    auto udp_socket = UDPsocket("239.0.0.1", 12345);

    uint64_t seq_num = 0;
    bool is_first_msg = true;
    uint64_t start_time = 0;
    uint64_t last_sent_time = 0;
    moldudp64::MessageLength msg_len = 0;
    itch::Timestamp first_timestamp = 0;
\
    while (ifstrm.read(reinterpret_cast<char *>(&msg_len), moldudp64::kMessageLengthSize)) {
        std::memcpy(msg_block_buf, &msg_len, sizeof(msg_len));
        msg_len = std::byteswap(msg_len);
        // read the message into the buffer
        ifstrm.read(msg_block_buf + moldudp64::kMessageLengthSize, msg_len);
        // timestamp network byte order -> machine byte order
        itch::Timestamp timestamp = 0;
        std::memcpy(&timestamp, msg_block_buf + moldudp64::kMessageLengthSize + itch::kTimestampOffset, itch::kTimestampSize);
        timestamp = std::byteswap(timestamp) >> ((sizeof(timestamp) - itch::kTimestampSize) * CHAR_BIT);
        if (timestamp < period_start) {
            continue;
        }
        if (timestamp > period_end) {
            break;
        }

        if (is_first_msg) {
            start_time = get_time_ns();
            last_sent_time = start_time;
            first_timestamp = timestamp;
            is_first_msg = false;
        } else {
            uint64_t now = 0;
            while ((now = get_time_ns()) - start_time < timestamp - first_timestamp) {
                if (now - last_sent_time > kMaxPacketInterval && packet_len > moldudp64::kHeaderSize) {
                    uint16_t msg_cnt_be = std::byteswap(msg_cnt);
                    uint64_t seq_num_be = std::byteswap(seq_num);
                    memcpy(packet_buf + moldudp64::kSessionSize, &seq_num_be, sizeof(seq_num_be));
                    memcpy(packet_buf + moldudp64::kSessionSize + moldudp64::kSequenceNumberSize, &msg_cnt_be, sizeof(msg_cnt_be));
                    udp_socket.send(packet_buf, packet_len);
                    msg_cnt = 0;
                    packet_len = moldudp64::kHeaderSize;
                    last_sent_time = now;
                    ++seq_num;
                }
            }
        }
        if (packet_len + moldudp64::kMessageLengthSize + msg_len > moldudp64::kMaxPayload
                && packet_len > moldudp64::kHeaderSize) {
            uint16_t msg_cnt_be = std::byteswap(msg_cnt);
            uint64_t seq_num_be = std::byteswap(seq_num);
            memcpy(packet_buf + moldudp64::kSessionSize, &seq_num_be, sizeof(seq_num_be));
            memcpy(packet_buf + moldudp64::kSessionSize + moldudp64::kSequenceNumberSize, &msg_cnt_be, sizeof(msg_cnt_be));
            udp_socket.send(packet_buf, packet_len);
            msg_cnt = 0;
            packet_len = moldudp64::kHeaderSize;
            last_sent_time = get_time_ns();
            ++seq_num;
        }
        std::memcpy(packet_buf + packet_len, msg_block_buf, msg_len + moldudp64::kMessageLengthSize);
        packet_len += msg_len + moldudp64::kMessageLengthSize;
        ++msg_cnt;

    }
    std::cout << seq_num << std::endl;
}

int main() {
    const auto period_start = hh_mm_to_ns_after_midnight(4, 40);
    const auto period_end = hh_mm_to_ns_after_midnight(4, 41);
    replay(kHistoricalDataPath, period_start, period_end);
}