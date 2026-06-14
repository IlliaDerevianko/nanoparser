#pragma once
#include <chrono>
#include <sys/poll.h>

#include "udp_socket.hpp"

namespace nanoparser {
    class Poller {
    public:
        explicit Poller(const UdpSocket& socket);
        void Poll(std::chrono::milliseconds timeout);

    private:
        pollfd fds_;
    };
}


