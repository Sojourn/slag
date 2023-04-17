#pragma once

#include <span>
#include <optional>
#include <cstdint>
#include <cstddef>

namespace slag {

    class BufferReader {
    public:
        explicit BufferReader(std::span<const std::byte> buffer);

        std::span<const std::byte> buffer() const;
        [[nodiscard]] size_t offset() const;
        [[nodiscard]] size_t readable_size_bytes() const;

        std::optional<std::byte> read();
        std::optional<std::span<const std::byte>> read(size_t size_bytes);

        std::byte read_unchecked();
        std::span<const std::byte> read_unchecked(size_t size_bytes);

    private:
        std::span<const std::byte> buffer_;
        size_t                     offset_;
    };

}

#include "buffer_reader.hpp"
