#include "reader_mmap.hpp"

#include <bit>
#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <stdexcept>
#include <unistd.h>
#include <utility>
#include <sys/mman.h>
#include <sys/stat.h>

#include "itch.hpp"


namespace nanoparser {
    ItchFeedReaderMmap::ItchFeedReaderMmap(const std::string& path) : offset_(0) {
        int fd = open(path.c_str(), O_RDONLY);
        if (fd == -1)
            throw std::runtime_error("Failed to open the file");

        struct stat sb{};
        if (fstat(fd, &sb) == -1) {
            close(fd);
            throw std::runtime_error("fstat failed");
        }
        auto size = static_cast<size_t>(sb.st_size);

        auto data = static_cast<char*>(
            mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0)
        );
        close(fd);
        if (data == MAP_FAILED)
            throw std::runtime_error("mmap failed");
        madvise(data, size, MADV_SEQUENTIAL);
        contents = std::span{data, size};
    }

    ItchFeedReaderMmap::ItchFeedReaderMmap(ItchFeedReaderMmap&& other) noexcept : offset_{} {
        move_from(other);
    }

    ItchFeedReaderMmap& ItchFeedReaderMmap::operator=(ItchFeedReaderMmap&& other) noexcept {
        if (this != &other) {
            if (!locked_.empty())
                munlock(locked_.data(), locked_.size());
            if (!contents.empty())
                munmap(contents.data(), contents.size());
            move_from(other);
        }
        return *this;
    }

    ItchFeedReaderMmap::~ItchFeedReaderMmap() {
        if (!locked_.empty())
            munlock(locked_.data(), locked_.size());
        if (!contents.empty())
            munmap(contents.data(), contents.size());
    }

    std::span<const char> ItchFeedReaderMmap::read() {
        if (offset_ == contents.size())
            return {};
        if (contents.size() - offset_ < Itch::kMessageLengthSize)
            throw std::runtime_error("Itch feed file is malformed");

        auto offset_old = offset_;
        uint16_t msg_len_be = 0;
        memcpy(&msg_len_be, contents.data() + offset_, Itch::kMessageLengthSize);
        offset_ += Itch::kMessageLengthSize;

        auto msg_len_le = std::byteswap(msg_len_be);
        if (contents.size() - offset_ < msg_len_le)
            throw std::runtime_error("Itch feed file is malformed");
        offset_ += msg_len_le;

        return {contents.data() + offset_old, static_cast<uint16_t>(msg_len_le + Itch::kMessageLengthSize)};
    }

    void ItchFeedReaderMmap::prefetch() {
        long page_size = sysconf(_SC_PAGESIZE);

        auto unaligned_numeric = reinterpret_cast<uintptr_t>(contents.data() + offset_);

        uintptr_t aligned_numeric = unaligned_numeric & ~(static_cast<uintptr_t>(page_size) - 1);
        auto aligned = reinterpret_cast<char*>(aligned_numeric);
        auto size =
                std::min(kMemLockMaxSize, (contents.size() - offset_) & ~(static_cast<size_t>(page_size) - 1));

        int res = mlock(aligned, size);
        if (res == -1) {
            std::cout << "Failed to memlock" << std::endl;
            return;
        }
        locked_ = std::span{aligned, size};
    }

    void ItchFeedReaderMmap::move_from(ItchFeedReaderMmap& other) {
        offset_ = std::exchange(other.offset_, 0);
        contents = std::exchange(other.contents, std::span<char>{});
        locked_ = std::exchange(other.locked_, std::span<char>{});
    }
}
