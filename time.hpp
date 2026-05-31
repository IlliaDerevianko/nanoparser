#pragma once
#include <cstdint>


namespace nanoparser::time {
    constexpr uint64_t sec_to_nsec = 1'000'000'000;
    constexpr uint64_t ms_to_ns = 1'000'000;
    constexpr uint64_t secs_to_ms = 1000;
    constexpr unsigned mins_to_secs = 60;
    constexpr unsigned hrs_to_mins = 60;

    uint64_t get_time_ns();
    uint64_t to_ns_after_midnight(unsigned hh, unsigned mm, unsigned ss = 0, unsigned ms = 0);
}


