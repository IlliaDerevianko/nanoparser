#include "receiver.hpp"
#include "src/networking/rx_udp_socket.hpp"

int main() {
    using namespace std::chrono_literals;
    nanoparser::Receiver receiver;
    receiver.Receive(500ms);
    receiver.DumpTimestamps();
}
