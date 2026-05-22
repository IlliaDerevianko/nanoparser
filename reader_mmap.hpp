#pragma once
#include <memory>
#include <span>
#include <string_view>

#include "read_result.hpp"


namespace nanoparser {
    class ItchFeedReaderMmap {
    public:
        using Position = size_t;
        explicit ItchFeedReaderMmap(const std::string& path);
        ItchFeedReaderMmap(const ItchFeedReaderMmap&) = delete;
        ItchFeedReaderMmap operator=(const ItchFeedReaderMmap&) = delete;
        ItchFeedReaderMmap(ItchFeedReaderMmap&&) noexcept ;
        ItchFeedReaderMmap& operator=(ItchFeedReaderMmap&&) noexcept;
        ~ItchFeedReaderMmap();

        ReadResult<ItchFeedReaderMmap> read();
        void prefetch();
        void seek(ReaderCheckpoint<ItchFeedReaderMmap> checkpoint);

    private:
        static constexpr size_t kMemLockMaxSize = 1 << 25;
        std::span<char> contents;
        std::span<char> mem_locked_;
        Position offset_;
        std::shared_ptr<void> sentinel_;

        void move_from(ItchFeedReaderMmap& other);
    };
}
