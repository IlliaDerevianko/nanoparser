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

    private:
        char* data_;
        size_t size_;
        size_t offset_;
    };
}
