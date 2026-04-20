#include <fstream>
#include <string>

#include "replayer.hpp"
#include "time.hpp"

int main() {
    const std::string kHistoricalDataPath =
        "/home/illiaderevianko/Desktop/projects/nanoparser/data/01302019.NASDAQ_ITCH50";

    nanoparser::Replayer<nanoparser::ItchFeedReaderMmap> replayer(kHistoricalDataPath);
    const auto period_start = nanoparser::time::hh_mm_ss_to_ns_after_midnight(16, 00, 00);
    const auto period_end = nanoparser::time::hh_mm_ss_to_ns_after_midnight(16, 00, 01);
    replayer.replay(period_start, period_end);
    return 0;
}