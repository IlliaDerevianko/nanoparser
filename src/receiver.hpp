#pragma once
#include <chrono>


namespace nanoparser {
    class Receiver {
    public:
        void Receive(std::chrono::milliseconds timeout);
        void DumpTimestamps() const;
    private:
        std::vector<uint64_t> timestamps_;
    };
}
