#include <filesystem>
#include <string>

#include "config.hpp"
#include "replayer.hpp"
#include "time.hpp"
#include "dotenv.h"

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    unsigned hh_start, mm_start, ss_start,
        hh_end, mm_end, ss_end;
    if (argc != 3
        || sscanf(argv[1], "%u:%u:%u", &hh_start, &mm_start, &ss_start) != 3
        || sscanf(argv[2], "%u:%u:%u", &hh_end, &mm_end, &ss_end) != 3) {
        throw std::runtime_error("Expected format of arguments is hh:mm:ss");
    }
    dotenv::init();
    fs::path root = dotenv::getenv("ROOT");
    fs::path replay_data_path = root / "data" / "01302019.NASDAQ_ITCH50";
    nanoparser::ReplayerConfig config = {
        .max_msg_delay = nanoparser::config::kMaxMessageDelayNs,
        .multicast_ip = dotenv::getenv("MULTICAST_IP"),
        .src_ip = dotenv::getenv("SRC_IP"),
        .dst_port = nanoparser::config::GetDstPort(),
        .tx_interface = dotenv::getenv("TX_INTERFACE")
    };
    nanoparser::Replayer<nanoparser::ItchFeedReaderMmap> replayer(replay_data_path, config);
    const auto period_start = nanoparser::time::to_ns_after_midnight(hh_start, mm_start, ss_start);
    const auto period_end = nanoparser::time::to_ns_after_midnight(hh_end, mm_end, ss_end);
    replayer.replay(period_start, period_end);
    return 0;
}
