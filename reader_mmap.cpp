#include "reader_mmap.hpp"

#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <stdexcept>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "itch.hpp"


namespace nanoparser {
    ItchFeedReaderMmap::ItchFeedReaderMmap(const std::string& path) : data_(nullptr), size_{}, offset_(0) {
        int fd = open(path.c_str(), O_RDONLY);
        if (fd == -1)
            throw std::runtime_error("Failed to open the file");

        struct stat sb{};
        if (fstat(fd, &sb) == -1) {
            close(fd);
            throw std::runtime_error("fstat failed");
        }
        size_ = static_cast<size_t>(sb.st_size);

        data_ = static_cast<char*>(
            mmap(nullptr, size_, PROT_READ, MAP_PRIVATE, fd, 0)
        );
        madvise(data_, size_, MADV_SEQUENTIAL);
        close(fd);
        if (data_ == MAP_FAILED)
            throw std::runtime_error("mmap failed");
    }

    ItchFeedReaderMmap::ItchFeedReaderMmap(ItchFeedReaderMmap&& other) noexcept {
        data_ = other.data_;
        size_ = other.size_;
        offset_ = other.offset_;
        other.data_ = nullptr;
        other.size_ = 0;
        other.offset_ = 0;
    }

    ItchFeedReaderMmap& ItchFeedReaderMmap::operator=(ItchFeedReaderMmap&& other) noexcept {
        if (this != &other) {
            if (data_ != nullptr)
                munmap(data_, size_);
            data_ = other.data_;
            size_ = other.size_;
            offset_ = other.offset_;
            other.data_ = nullptr;
            other.size_ = 0;
            other.offset_ = 0;
        }
        return *this;
    }

    std::span<const char> ItchFeedReaderMmap::read() {
        if (offset_ == size_)
            return {};
        if (size_ - offset_ < Itch::kMessageLengthSize)
            throw std::runtime_error("Itch feed file is malformed");
        auto offset_old = offset_;
        uint16_t msg_len_be = 0;
        memcpy(&msg_len_be, data_ + offset_, Itch::kMessageLengthSize);
        offset_ += Itch::kMessageLengthSize;
        auto msg_len_le = std::byteswap(msg_len_be);
        if (size_ - offset_ < msg_len_le)
            throw std::runtime_error("Itch feed file is malformed");
        offset_ += msg_len_le;
        return {data_ + offset_old, static_cast<uint16_t>(msg_len_le + Itch::kMessageLengthSize)};
    }

    ItchFeedReaderMmap::~ItchFeedReaderMmap() {
        if (data_ != nullptr)
            munmap(data_, size_);
    }
}
