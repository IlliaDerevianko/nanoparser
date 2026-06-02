#include <fstream>
#include <string>
#include <sched.h>

#include "replayer.hpp"
#include "time.hpp"

int main() {
    #ifdef __linux__
        cpu_set_t set;
        CPU_ZERO(&set);
        CPU_SET(5, &set);
        sched_setaffinity(0, sizeof(set), &set);

        sched_param param = { .sched_priority = 99 };
        sched_setscheduler(0, SCHED_FIFO, &param);
    #endif

    const std::string kHistoricalDataPath =
        "/home/illia-derevianko/Projects/nanoparser/data/01302019.NASDAQ_ITCH50";

    nanoparser::Replayer<nanoparser::ItchFeedReaderMmap> replayer(kHistoricalDataPath);
    const auto period_start = nanoparser::time::to_ns_after_midnight(4, 0, 0);
    const auto period_end = nanoparser::time::to_ns_after_midnight(4, 0, 10);
    replayer.replay(period_start, period_end);
    return 0;
}
