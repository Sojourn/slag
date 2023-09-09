#include "slag/postal/buffer_slice.h"
#include <cassert>

namespace slag::postal {

    BufferSlice::BufferSlice(BufferHandle buffer, BufferCursor first, BufferCursor last)
        : buffer_{std::move(buffer)}
        , first_{first}
        , last_{last}
    {
        assert(buffer_);
        assert(first_.buffer_offset() <= last_.buffer_offset());
    }

    BufferSlice::operator bool() const {
        return !is_empty();
    }

    bool BufferSlice::is_empty() const {
        return size() == 0;
    }

    size_t BufferSlice::size() const {
        return last_.buffer_offset() - first_.buffer_offset();
    }

    BufferHandle& BufferSlice::buffer() {
        return buffer_;
    }

    BufferCursor BufferSlice::begin() {
        return first_;
    }

    BufferCursor BufferSlice::end() {
        return last_;
    }

    BufferSlice BufferSlice::first(size_t size) {
        return middle(0, size);
    }

    BufferSlice BufferSlice::middle(size_t relative_offset, size_t size) {
        BufferCursor first = first_;
        first.advance(relative_offset);

        BufferCursor last = first;
        last.advance(size);

        return BufferSlice {
            buffer_.share(),
            first,
            last
        };
    }

    BufferSlice BufferSlice::last(size_t size) {
        if (size_bytes_ >= size()) {
            return *this;
        }

        return middle(size() - size, size);
    }

}
