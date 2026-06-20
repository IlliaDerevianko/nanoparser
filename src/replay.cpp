#include <filesystem>
#include <string>

#include "config.hpp"
#include "replayer.hpp"
#include "time.hpp"
#include "dotenv.h"

namespace fs = std::filesystem;

int main() {
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
    const auto period_start = nanoparser::time::to_ns_after_midnight(4, 0, 0);
    const auto period_end = nanoparser::time::to_ns_after_midnight(4, 0, 5);
    replayer.replay(period_start, period_end);
    return 0;
}
