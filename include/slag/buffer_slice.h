#pragma once

#include <span>
#include "slag/handle.h"
#include "slag/buffer.h"

namespace slag {

    // (Potentially) holds a reference to a buffer, and a selection of part of it.
    class BufferSlice {
    public:
        BufferSlice() = default;
        BufferSlice(Handle<Buffer> buffer);
        BufferSlice(Handle<Buffer> buffer, std::span<const std::byte> data);

        [[nodiscard]] bool is_empty() const;
        [[nodiscard]] size_t offset() const;
        [[nodiscard]] size_t length() const;
        [[nodiscard]] const Handle<Buffer>& buffer() const;
        [[nodiscard]] std::span<const std::byte> data() const;

    private:
        Handle<Buffer>             buffer_;
        std::span<const std::byte> data_;
    };

}
