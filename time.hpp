#pragma once
#include <cstdint>


namespace nanoparser::time {
    constexpr uint64_t sec_to_nsec = 1'000'000'000;
    constexpr unsigned mins_to_secs = 60;
    constexpr unsigned hrs_to_mins = 60;

    uint64_t get_time_ns();
    uint64_t hh_mm_to_ns_after_midnight(unsigned hh, unsigned mm);
}


