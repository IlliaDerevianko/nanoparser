#include "time.hpp"

#include <cstdint>
#include <ctime>

namespace nanoparser::time {
    uint64_t get_time_ns() {
        std::timespec ts{};
        clock_gettime(CLOCK_MONOTONIC, &ts);
        return timespec_to_ns(ts);
    }

    uint64_t to_ns_after_midnight(const unsigned hh, const unsigned mm, const unsigned ss, const unsigned ms) {
        return (((hh * hrs_to_mins + mm) * mins_to_secs + ss) * secs_to_ms + ms) * ms_to_ns;
    }

    uint64_t timespec_to_ns(const timespec ts) {
        return sec_to_nsec * static_cast<uint64_t>(ts.tv_sec) + static_cast<uint64_t>(ts.tv_nsec);
    }
}
