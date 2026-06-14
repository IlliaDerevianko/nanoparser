#include "null_tx_udp_socket.hpp"


#include <span>


namespace nanoparser {


    void NullTxUdpSocket::Send([[maybe_unused]] std::span<const char> buf) {
        // no-op
    }

}


