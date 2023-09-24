#pragma once

#include "slag/postal/buffer.h"
#include "slag/postal/buffer_ledger.h"
#include <cstdint>
#include <cstddef>

namespace slag {

    class BufferCursor {
    public:
        BufferCursor(BufferSegment& segment);
        BufferCursor(const BufferCursor&) = default;
        BufferCursor(BufferCursor&&) = default;
        ~BufferCursor() = default;

        BufferCursor& operator=(const BufferCursor&) = default;
        BufferCursor& operator=(BufferCursor&&) = default;

        BufferSegment* segment();
        size_t segment_offset() const;
        size_t buffer_offset() const;

        // Remaining data in the current segment.
        std::span<std::byte> content();

        size_t advance();
        size_t advance(size_t count);

        bool operator==(const BufferCursor& that) const;
        bool operator!=(const BufferCursor& that) const;

    private:
        BufferSegment* segment_;
        size_t         segment_offset_;
        size_t         buffer_offset_;
    };

}
