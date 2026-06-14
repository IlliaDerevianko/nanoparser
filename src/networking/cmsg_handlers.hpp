#pragma once

#include <cstdint>
#include <sys/socket.h>

namespace nanoparser {
    struct TimestampExtractor {
        uint64_t timestamp;

        bool operator()(const cmsghdr* cmsg);
    };
}
