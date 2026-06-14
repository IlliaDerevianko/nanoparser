#pragma once
#include <chrono>

#include "src/networking/udp_socket.hpp"

namespace nanoparser {
    class Receiver {
    public:
        void Receive(std::chrono::milliseconds timeout);
        void DumpTimestamps() const;
    private:
        std::vector<uint64_t> timestamps_;
    };
}
