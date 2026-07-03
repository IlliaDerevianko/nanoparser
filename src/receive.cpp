#include "config.hpp"
#include "dotenv.h"
#include "receiver.hpp"
#include "networking/rx_udp_socket.hpp"

int main() {
    using namespace std::chrono_literals;
    dotenv::init();
    if (const std::string cpu_set_str = dotenv::getenv("RECEIVE_CPU_SET"); !cpu_set_str.empty()) {
        cpu_set_t set;
        CPU_ZERO(&set);
        CPU_SET(std::stoi(cpu_set_str), &set);
        if (sched_setaffinity(0, sizeof(set), &set) == -1) {
            std::cerr << "Failed to set affinity" << std::endl;
        }
    }
    if (const std::string priority_str = dotenv::getenv("RECEIVE_PRIORITY"); !priority_str.empty()) {
        sched_param param = { .sched_priority = std::stoi(priority_str) };
        if (sched_setscheduler(0, SCHED_FIFO, &param) == -1) {
            std::cerr << "Failed to set priority" << std::endl;
        }
    }

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
