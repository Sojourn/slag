#include "slag/byte_stream.h"
#include <cstring>
#include <cassert>

void slag::ByteStream::write(std::span<const std::byte> data) {
    if (data.empty()) {
        return;
    }

    // Segment segment;
    // segment.relative_producer_sequence = data.size_bytes();

    (void)data;
}

std::span<const std::byte> slag::ByteStream::read(size_t count) {
    (void)count;

    return {};
}
