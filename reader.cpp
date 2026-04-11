#include "reader.hpp"

#include <fstream>
#include <stdexcept>

#include "itch.hpp"
#include "moldudp64.hpp"

namespace nanoparser {
    ItchFeedReader::ItchFeedReader(std::string_view path) : file_(path.data(), std::ios::binary) {
        if (!file_.is_open())
            throw std::runtime_error("Failed to open the file");
    }

    uint16_t ItchFeedReader::read(char *buf) {
        if (!file_.read(buf, Itch::kMessageLengthSize))
            return 0;
        uint16_t msg_len_be = 0;
        memcpy(&msg_len_be, buf, Itch::kMessageLengthSize);
        auto msg_len_le = std::byteswap(msg_len_be);
        if (!file_.read(buf + Itch::kMessageLengthSize, msg_len_le))
            throw std::runtime_error("Itch feed file is malformed.");
        return msg_len_le + Itch::kMessageLengthSize;
    }
}
