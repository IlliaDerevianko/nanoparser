#pragma once
#include <limits>
#include <string>

#include "moldudp64.hpp"
#include "networking.hpp"
#include "reader.hpp"

namespace nanoparser {
    struct ReplayerConfig {
        uint64_t max_inter_packet_interval = 10'000'000;
        std::string ip = "239.0.0.1";
        std::uint16_t port = 12345;
    };

    class Replayer {
    public:
        explicit Replayer(std::string_view data_src_path, const ReplayerConfig& config = ReplayerConfig{});
        void replay(uint64_t start = 0, uint64_t end = std::numeric_limits<uint64_t>::max());

    private:
        ItchFeedReader reader_;
        UDPsocket socket_;
        uint64_t max_inter_packet_interval_;
        uint64_t first_timestamp_;
        uint64_t start_time_;
        uint64_t last_sent_time_;
        uint64_t seq_num_;
        char packet_buf_[MoldUDP64Packet::kMaxSize];
        MoldUDP64Packet mold_udp64_packet_;

        void consume_first_msg_after(uint64_t start);

        void send();
    };
}


