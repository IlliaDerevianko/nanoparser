#include "poller.hpp"


namespace nanoparser {
    Poller::Poller(const UdpSocket& socket) : fds_{} {
        fds_.fd = socket.Fd();
    }

    void Poller::Poll(std::chrono::milliseconds timeout) {
        if (poll(&fds_, 1, static_cast<int>(timeout.count())) == -1) {
            throw std::runtime_error("failed to poll");
        }
    }
}
