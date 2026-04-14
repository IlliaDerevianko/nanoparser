#include "time.hpp"

#include <cstdint>
#include <ctime>

namespace nanoparser::time {
    uint64_t get_time_ns() {
        std::timespec ts{};
        clock_gettime(CLOCK_MONOTONIC, &ts);
        return sec_to_nsec * static_cast<uint64_t>(ts.tv_sec) + static_cast<uint64_t>(ts.tv_nsec);
    }

    uint64_t hh_mm_ss_to_ns_after_midnight(const unsigned hh, const unsigned mm, const unsigned ss) {
        return ((hh * hrs_to_mins + mm) * mins_to_secs + ss) * sec_to_nsec;
    }
}
