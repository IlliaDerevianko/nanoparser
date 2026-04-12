#pragma once
#include <fstream>

#include "itch.hpp"

namespace nanoparser {
    class ItchFeedReaderStream {
    public:
        explicit ItchFeedReaderStream(const std::string& path);

        std::span<const char> read();

    private:
        std::ifstream file_;
        std::array<char, Itch::kMaxPacketSize> buf_;
    };
}
