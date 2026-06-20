#pragma once
#include <cstddef>
#include <cstring>
#include <sys/socket.h>

#include "../constants.hpp"
// TODO: consider move/copy semantics
namespace nanoparser {
    template<std::size_t DataBufSize = networking::kMaxUdpPayload, std::size_t CtrlBufSize = 512>
    class MessageHeader {
    public:
        MessageHeader() : msg_{}, data_buf_{}, ctrl_buf_{} {
            iov_ = {data_buf_, DataBufSize};
            msg_.msg_iov = &iov_;
            msg_.msg_iovlen = 1;
            msg_.msg_control = ctrl_buf_;
            msg_.msg_controllen = CtrlBufSize;
        }

        void ResetCtrlBuf() {
            memset(&ctrl_buf_, 0, CtrlBufSize);
            msg_.msg_controllen = CtrlBufSize;
        }

        [[nodiscard]] msghdr* data() {
            return &msg_;
        }

        template <typename CmsgHandler>
        void ForEachCmsg(CmsgHandler&& handler) {
            for (cmsghdr* cmsg = CMSG_FIRSTHDR(&msg_); cmsg != nullptr; cmsg = CMSG_NXTHDR(&msg_, cmsg)) {
                handler(cmsg);
            }
        }

    private:
        msghdr msg_;
        char data_buf_[DataBufSize];
        iovec iov_;
        alignas(cmsghdr) char ctrl_buf_[CtrlBufSize];
    };

    template <std::size_t CtrlBufSize = 512>
    using MessageHeaderNoPayload = MessageHeader<1, CtrlBufSize>;
}


