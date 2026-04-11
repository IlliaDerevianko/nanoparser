#pragma once
#include <fstream>
#include <string_view>

namespace nanoparser {
    class ItchFeedReader {
    public:
        explicit ItchFeedReader(std::string_view path);

        uint16_t read(char *buf);

    private:
        std::ifstream file_;
    };
}
