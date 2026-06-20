#include "config.hpp"
#include "dotenv.h"
#include "receiver.hpp"
#include "networking/rx_udp_socket.hpp"

int main() {
    using namespace std::chrono_literals;
    dotenv::init();
    nanoparser::ReceiverConfig config = {
        .timeout = 500ms,
        .multicast_ip = dotenv::getenv("MULTICAST_IP"),
        .dst_ip = dotenv::getenv("DST_IP"),
        .dst_port = nanoparser::config::GetDstPort(),
        .rx_interface = dotenv::getenv("RX_INTERFACE"),
    };
    nanoparser::Receiver receiver(config);
    receiver.Receive();
    receiver.DumpTimestamps();
}
