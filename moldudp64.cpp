#include "moldudp64.hpp"

#include <cassert>
#include <ostream>


namespace nanoparser {
    MoldUDP64Packet::MoldUDP64Packet(char* buf) : buf_(buf), msg_cnt_(0), size_(kHeaderSize) {}

    void MoldUDP64Packet::set_session(std::span<const char, kSessionSize> id) const {
        memcpy(buf_, id.data(), kSessionSize);
    }

    void MoldUDP64Packet::set_seq_num(const uint64_t seq_num) const {
        const uint64_t seq_num_be = std::byteswap(seq_num);
        memcpy(buf_ + kSessionSize, &seq_num_be, sizeof(seq_num_be));
    }

    void MoldUDP64Packet::append_msg(std::span<const char> msg_buf) {
        assert(size_ + msg_buf.size() <= kMaxSize);
        memcpy(buf_ + size_, msg_buf.data(), msg_buf.size());
        size_ += static_cast<uint16_t>(msg_buf.size());
        ++msg_cnt_;
    }

    size_t MoldUDP64Packet::bytes_remaining() const {
        return  kMaxSize - size_;
    }

    bool MoldUDP64Packet::empty() const {
        return size_ == kHeaderSize;
    }

    uint16_t MoldUDP64Packet::send(UDPsocket& udp_socket) {
        const uint16_t msg_cnt_be = std::byteswap(msg_cnt_);
        memcpy(buf_ + kSessionSize + kSequenceNumberSize, &msg_cnt_be, sizeof(msg_cnt_be));
        udp_socket.send({buf_, size_});
        auto sent = msg_cnt_;
        msg_cnt_ = 0;
        size_ = kHeaderSize;
        return sent;
    }
}
