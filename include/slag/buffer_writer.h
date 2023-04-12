#pragma once

#include <span>
#include <cstdint>
#include <cstddef>

namespace slag {

    class BufferWriter {
    public:
        explicit BufferWriter(std::span<std::byte> buffer);

        [[nodiscard]] std::span<std::byte> buffer();
        [[nodiscard]] std::span<const std::byte> buffer() const;
        [[nodiscard]] size_t offset() const;
        [[nodiscard]] size_t writable_size_bytes() const;

        void write(std::byte data);
        void write(std::span<const std::byte> data);

        void write_unchecked(std::byte data);
        void write_unchecked(std::span<const std::byte> data);

    private:
        std::span<std::byte> buffer_;
        size_t               offset_;
    };

}

#include "buffer_writer.hpp"
