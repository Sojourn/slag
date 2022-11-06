#include "slag/buffer.h"

slag::Buffer::Buffer(size_t capacity) {
    data_.resize(capacity);
}

slag::Buffer::Buffer(std::span<const std::byte> data) {
    data_.insert(data_.end(), data.begin(), data.end());
}

std::span<std::byte> slag::Buffer::data() {
    return std::span{data_.data(), data_.size()};
}

std::span<const std::byte> slag::Buffer::data() const {
    return std::span{data_.data(), data_.size()};
}
