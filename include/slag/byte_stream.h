#pragma once

#include <span>
#include <vector>
#include <cstdint>
#include <cstddef>
#include <cassert>
#include "slag/handle.h"
#include "slag/buffer.h"
#include "slag/buffer_slice.h"

namespace slag {

    // TODO: expose readable/writable events
    class ByteStream {
    public:
        // lifetime of the result lasts until the returned buffer handle is discarded.
        [[nodiscard]] BufferSlice read_stable(size_t count);
        [[nodiscard]] size_t readable_byte_count() const;
        [[nodiscard]] std::span<const std::byte> read(size_t count);

        // undoes the previous read
        [[nodiscard]] size_t unread(size_t count);

        [[nodiscard]] BufferSlice peek_stable(size_t count);
        [[nodiscard]] std::span<const std::byte> peek(size_t count);

        void write(std::span<const std::byte> data);
        void write(BufferSlice buffer_slice);

    private:
        struct Segment {
            Handle<Buffer> buffer;
            bool           frozen                     = false;
            uint64_t       relative_consumer_sequence = 0;
            uint64_t       relative_producer_sequence = 0;

            [[nodiscard]] std::span<std::byte> consumer_data();
            [[nodiscard]] std::span<std::byte> producer_data();
            [[nodiscard]] size_t write(std::span<const std::byte> data);
        };

        uint64_t             absolute_producer_sequence_ = 0;
        uint64_t             absolute_consumer_sequence_ = 0;
        std::vector<Segment> segments_;
    };

}
