#pragma once
#include <cstdint>
#include <limits>

#include "dotenv.h"


namespace nanoparser::config {
    constexpr uint64_t kMaxMessageDelayNs = 10'000'000;

    uint16_t inline GetDstPort() {
        dotenv::init();
        int port = std::stoi(dotenv::getenv("DST_PORT"));
        if (port < std::numeric_limits<uint16_t>::min() || port > std::numeric_limits<uint16_t>::max())
            throw std::runtime_error("Destination port number is out of range");
        return static_cast<uint16_t>(port);
    }
}
