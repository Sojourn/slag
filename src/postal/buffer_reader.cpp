#include "slag/postal/buffer_reader.h"
#include "slag/core/domain.h"
#include <cassert>

namespace slag {

    BufferReader::BufferReader(BufferHandle handle)
        : handle_{std::move(handle)}
        , entry_{nation().buffer_ledger().get_national_entry(handle_.descriptor())}
        , segment_{}
        , storage_{}
        , position_{}
    {
        // Initialize position and the current segment.
        seek(0);
    }

    size_t BufferReader::size() const {
        return entry_.size;
    }

    size_t BufferReader::tell() const {
        return position_;
    }

    void BufferReader::seek(size_t position) {
        if (entry_.size <= position) {
            throw std::runtime_error("Invalid buffer position");
        }

        // Reset position to the start of the buffer.
        segment_ = &entry_.head;
        storage_ = std::span{static_cast<const std::byte*>(segment_->data), segment_->capacity};
        position_ = position;

        advance(position - position_);
    }

    void BufferReader::advance(size_t count) {
        while (count) {
            count -= read(count).size_bytes();
        }
    }

    // O(1), zero-copy
    std::span<const std::byte> BufferReader::read(size_t count) {
        if (storage_.empty()) {
            if (!segment_) {
                return {};
            }

            segment_ = segment_->next;
            storage_ = std::span{static_cast<const std::byte*>(segment_->data), segment_->capacity};
        }

        // Reduce to the remaining number of bytes in the buffer.
        count = std::min(count, entry_.size - position_);

        // Reduce to the remaining number of bytes in the buffer segment.
        count = std::min(count, storage_.size_bytes());

        auto result = storage_.first(count);
        storage_ = storage_.subspan(count);
        position_ += count;
        return result;
    }

}
