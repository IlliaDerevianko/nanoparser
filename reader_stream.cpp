#include "reader_stream.hpp"

#include <fstream>
#include <stdexcept>

#include "itch.hpp"
#include "moldudp64.hpp"

namespace nanoparser {
    ItchFeedReaderStream::ItchFeedReaderStream(const std::string& path)
        : file_(path.c_str()
        , std::ios::binary)
        , buf_{}
    {
        if (!file_.is_open())
            throw std::runtime_error("Failed to open the file");
    }

    std::span<const char> ItchFeedReaderStream::read() {
        if (!file_.read(buf_.data(), Itch::kMessageLengthSize))
            return {};
        uint16_t msg_len_be = 0;
        memcpy(&msg_len_be, buf_.data(), Itch::kMessageLengthSize);
        auto msg_len_le = std::byteswap(msg_len_be);
        if (!file_.read(buf_.data() + Itch::kMessageLengthSize, msg_len_le))
            throw std::runtime_error("Itch feed file is malformed");
        return {buf_.data(), static_cast<uint16_t>(msg_len_le + Itch::kMessageLengthSize)};
    }
}
