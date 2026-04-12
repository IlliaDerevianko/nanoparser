#include <fstream>
#include <string>

#include "replayer.hpp"
#include "time.hpp"

int main() {
    const std::string kHistoricalDataPath =
        "/home/illiaderevianko/Desktop/projects/nanoparser/data/01302019.NASDAQ_ITCH50";

    nanoparser::Replayer<nanoparser::ItchFeedReaderMmap> replayer(kHistoricalDataPath);
    const auto period_start = nanoparser::time::hh_mm_to_ns_after_midnight(4, 40);
    const auto period_end = nanoparser::time::hh_mm_to_ns_after_midnight(4, 41);
    replayer.replay(period_start, period_end);
    return 0;
}