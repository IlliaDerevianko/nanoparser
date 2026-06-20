#pragma once
#include <array>
#include <filesystem>
#include <fstream>

#include "itch.hpp"
#include "read_result.hpp"

namespace nanoparser {
    class ItchFeedReaderStream {
    public:
        using Position = std::streampos;
        explicit ItchFeedReaderStream(const std::filesystem::path& path);

        ReadResult<ItchFeedReaderStream> read();
        void prefetch();
        void seek(const ReaderCheckpoint<ItchFeedReaderStream>& checkpoint);
        [[nodiscard]] ReaderCheckpoint<ItchFeedReaderStream> get_begin_checkpoint() const;
        [[nodiscard]] ReaderCheckpoint<ItchFeedReaderStream> get_current_checkpoint();

    private:
        std::ifstream file_;
        std::array<char, Itch::kMaxPacketSize> buf_;
        std::shared_ptr<void> sentinel_;
        Position begin_;
    };
}
