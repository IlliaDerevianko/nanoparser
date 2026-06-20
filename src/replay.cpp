#include <fstream>
#include <string>

#include "replayer.hpp"
#include "time.hpp"

int main() {
    const std::string kHistoricalDataPath =
        "/home/illia-derevianko/Projects/nanoparser/data/01302019.NASDAQ_ITCH50";

    nanoparser::Replayer<nanoparser::ItchFeedReaderMmap> replayer(kHistoricalDataPath);
    const auto period_start = nanoparser::time::to_ns_after_midnight(4, 0, 0);
    const auto period_end = nanoparser::time::to_ns_after_midnight(4, 0, 5);
    replayer.replay(period_start, period_end);
    return 0;
}
