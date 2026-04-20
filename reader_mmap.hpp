#pragma once
#include <span>
#include <string_view>


namespace nanoparser {
    class ItchFeedReaderMmap {
    public:
        explicit ItchFeedReaderMmap(const std::string& path);
        ItchFeedReaderMmap(const ItchFeedReaderMmap&) = delete;
        ItchFeedReaderMmap operator=(const ItchFeedReaderMmap&) = delete;
        ItchFeedReaderMmap(ItchFeedReaderMmap&&) noexcept ;
        ItchFeedReaderMmap& operator=(ItchFeedReaderMmap&&) noexcept;
        ~ItchFeedReaderMmap();

        std::span<const char> read();
        void prefetch();

    private:
        static constexpr size_t kMemLockMaxSize = 1 << 25;
        std::span<char> contents;
        std::span<char> locked_;
        size_t offset_;

        void move_from(ItchFeedReaderMmap& other);
    };
}
