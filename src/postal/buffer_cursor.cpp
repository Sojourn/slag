#include "slag/postal/buffer_cursor.h"
#include <cassert>

namespace slag::postal {

    BufferCursor::BufferCursor(BufferSegment& segment)
        : segment_{&segment}
        , segment_offset_{0}
        , buffer_offset{0}
    {
    }

    BufferSegment* BufferCursor::segment() {
        return segment_;
    }

    size_t BufferCursor::segment_offset() const {
        return segment_offset_;
    }

    size_t BufferCursor::buffer_offset() const {
        return buffer_offset_;
    }

    std::span<std::byte> BufferCursor::content() {
        if (!segment_) {
            return {};
        }
        if (segment_->capacity == segment_offset_) {
            return {};
        }

        return segment_->data().subspan(segment_offset_);
    }

    size_t BufferCursor::advance() {
        if (segment_ == nullptr) {
            return 0;
        }

        size_t count = segment_->capacity - segment_offset_;
        segment_ = segment_->next;
        segment_offset_ += count;
        buffer_offset_ += count;

        return count;
    }

    size_t BufferCursor::advance(size_t count) {
        size_t remainder = count;

        while (segment_ && remainder) {
            if (segment_->capacity == segment_offset_) {
                segment_ = segment_->next;
                segment_offset_ = 0;
            }
            else {
                size_t reduced_count = std::min(count, segment_->capacity - segment_offset_);
                count -= reduced_count;
                segment_offset_ += reduced_count;
                buffer_offset_ += reduced_count;
            }
        }

        return count - remainder;
    }

    bool BufferCursor::operator==(const BufferCursor& that) const {
        return buffer_offset_ == that.buffer_offset();
    }

    bool BufferCursor::operator!=(const BufferCursor& that) const {
        return !operator==(that);
    }

}
