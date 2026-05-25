#include "reader_stream.hpp"

#include <bit>
#include <fstream>
#include <stdexcept>

#include "itch.hpp"
#include "moldudp64.hpp"

namespace nanoparser {
    ItchFeedReaderStream::ItchFeedReaderStream(const std::string& path)
        : file_(path.c_str(), std::ios::binary)
        , buf_{}
        , sentinel_(std::make_shared<int>(0))
        , begin_(file_.tellg())
    {
        if (!file_.is_open())
            throw std::runtime_error("Failed to open the file");
    }

    ReadResult<ItchFeedReaderStream> ItchFeedReaderStream::read() {
        if (!file_.read(buf_.data(), Itch::kMessageLengthSize))
            return {};
        uint16_t msg_len_be = 0;
        memcpy(&msg_len_be, buf_.data(), Itch::kMessageLengthSize);
        auto msg_len_le = std::byteswap(msg_len_be);
        if (!file_.read(buf_.data() + Itch::kMessageLengthSize, msg_len_le))
            throw std::runtime_error("Itch feed file is malformed");

        ReaderCheckpoint<ItchFeedReaderStream> checkpoint{file_.tellg(), sentinel_};
        std::span<const char> data (buf_.data(), static_cast<uint16_t>(msg_len_le + Itch::kMessageLengthSize));
        return {data, checkpoint};
    }

    void ItchFeedReaderStream::prefetch() {
        // no-op
    }

    void ItchFeedReaderStream::seek(ReaderCheckpoint<ItchFeedReaderStream> checkpoint) {
        if (!checkpoint.is_valid())
            throw std::runtime_error("Use of checkpoint issued by a freed reader");
        if (checkpoint.sentinel_.lock() != sentinel_)
            throw std::runtime_error("Use of checkpoint issued by a different reader");

        file_.seekg(checkpoint.offset_);

    }

    ReaderCheckpoint<ItchFeedReaderStream> ItchFeedReaderStream::get_begin_checkpoint() const {
        return {begin_, sentinel_};
    }

}
