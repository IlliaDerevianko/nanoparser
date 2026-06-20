#pragma once
#include <span>

#include "reader_checkpoint.hpp"

template<typename Reader>
struct ReadResult {
    std::span<const char> bytes;
    ReaderCheckpoint<Reader> checkpoint;

    [[nodiscard]] bool ok() const {
        return !empty() && valid();
    }

    [[nodiscard]] bool empty() const {
        return bytes.empty();
    }

    [[nodiscard]] bool valid() const {
        return checkpoint.is_valid();
    }
};
