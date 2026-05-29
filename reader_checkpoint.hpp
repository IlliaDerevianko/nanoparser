#pragma once

#include <memory>

template<typename Reader>
class ReaderCheckpoint {
    friend Reader;
public:
    ReaderCheckpoint(): offset_{} {

    }

    [[nodiscard]] bool is_valid() const {
        return !sentinel_.expired();
    }

private:
    ReaderCheckpoint(Reader::Position offset, const std::shared_ptr<void>& sentinel)
        : offset_(offset)
        , sentinel_(sentinel)
    {
    }

    Reader::Position offset_;
    std::weak_ptr<void> sentinel_;
};

