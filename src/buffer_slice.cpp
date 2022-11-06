#include "slag/buffer_slice.h"
#include <utility>
#include <cassert>

inline bool contains(std::span<const std::byte> parent, std::span<const std::byte> child) {
    if (child.begin() < parent.begin()) {
        return false;
    }
    if (parent.end() < child.end()) {
        return false;
    }

    return true;
}

slag::BufferSlice::BufferSlice(Handle<Buffer> buffer)
    : buffer_{std::move(buffer)}
    , data_{buffer_ ? buffer_->data() : std::span<const std::byte>{}}
{
}

slag::BufferSlice::BufferSlice(Handle<Buffer> buffer, std::span<const std::byte> data)
    : buffer_{std::move(buffer)}
    , data_{data}
{
    assert(!buffer_ || contains(buffer_->data(), data_));
}

const slag::Handle<slag::Buffer>& slag::BufferSlice::buffer() const {
    return buffer_;
}

bool slag::BufferSlice::is_empty() const {
    return data_.empty();
}

size_t slag::BufferSlice::offset() const {
    if (!buffer_) {
        return 0;
    }

    return data_.data() - buffer_->data().data();
}

size_t slag::BufferSlice::length() const {
    return data_.size_bytes();
}

std::span<const std::byte> slag::BufferSlice::data() const {
    return data_;
}
