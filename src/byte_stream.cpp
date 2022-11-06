#include "slag/byte_stream.h"
#include <cstring>
#include <cassert>

slag::BufferSlice slag::ByteStream::read_stable(size_t count) {
    for (auto segment_it = segments_.begin(); segment_it != segments_.end(); ) {
        auto data = segment_it->consumer_data();
        if (data.empty()) {
            // we leave fully consumed segments for temporary stability
            segment_it = segments_.erase(segment_it);
            continue;
        }

        count = std::min(count, data.size_bytes());
        data = data.last(count);

        segment_it->relative_consumer_sequence += count;
        absolute_consumer_sequence_ += count;

        return {segment_it->buffer, data};
    }

    return {};
}

size_t slag::ByteStream::readable_byte_count() const {
    return absolute_producer_sequence_ - absolute_consumer_sequence_;
}

std::span<const std::byte> slag::ByteStream::read(size_t count) {
    return read_stable(count).data();
}

size_t slag::ByteStream::unread(size_t count) {
    size_t remainder = count;
    for (auto it = segments_.rbegin(); it != segments_.rend(); ++it) {
        size_t relative_count = std::min(it->relative_consumer_sequence, remainder);

        it->relative_consumer_sequence -= relative_count;
        absolute_consumer_sequence_    -= relative_count;
        remainder                      -= relative_count;

        if (remainder == 0) {
            break; // early-exit
        }
    }

    return count - remainder;
}

slag::BufferSlice slag::ByteStream::peek_stable(size_t count) {
    auto buffer_slice = read_stable(count);
    if (!buffer_slice.is_empty()) {
        size_t bytes_unread = unread(buffer_slice.data().size_bytes());
        assert(bytes_unread == buffer_slice.data().size_bytes());
    }

    return buffer_slice;
}

std::span<const std::byte> slag::ByteStream::peek(size_t count) {
    return peek_stable(count).data();
}

void slag::ByteStream::write(std::span<const std::byte> data) {
    while (!data.empty()) {
        bool allocate_segment = false;
        if (segments_.empty()) {
            allocate_segment = true;
        }
        else {
            Segment& segment = segments_.back();
            if (segment.frozen || segment.producer_data().empty()) {
                allocate_segment = true;
            }
        }

        if (allocate_segment) {
            segments_.push_back(Segment {
                .buffer                     = make_handle<Buffer>(2048), // FIXME: use a buffer pool
                .frozen                     = false,
                .relative_consumer_sequence = 0,
                .relative_producer_sequence = 0,
            });
        }

        Segment& segment = segments_.back();

        size_t bytes_written = segment.write(data);
        absolute_consumer_sequence_ += bytes_written;
        data = data.last(data.size_bytes() - bytes_written);
    }
}

void slag::ByteStream::write(BufferSlice buffer_slice) {
    if (!buffer_slice || buffer_slice.is_empty()) {
        return;
    }

    size_t offset = buffer_slice.offset();
    size_t length = buffer_slice.length();

    segments_.push_back(Segment {
        .buffer                     = buffer_slice.buffer(),
        .frozen                     = true,
        .relative_consumer_sequence = offset,
        .relative_producer_sequence = length,
    });

    absolute_producer_sequence_ += length;
}

std::span<std::byte> slag::ByteStream::Segment::consumer_data() {
    auto data = buffer->data();
    return data.subspan(
        relative_consumer_sequence,
        relative_producer_sequence - relative_consumer_sequence
    );
}

std::span<std::byte> slag::ByteStream::Segment::producer_data() {
    auto data = buffer->data();
    return data.subspan(
        relative_producer_sequence,
        data.size_bytes() - relative_producer_sequence
    );
}

size_t slag::ByteStream::Segment::write(std::span<const std::byte> data) {
    if (frozen || data.empty()) {
        return 0;
    }

    auto src_data = data;
    auto dst_data = producer_data();
    auto cnt      = std::min(src_data.size_bytes(), dst_data.size_bytes());

    memcpy(dst_data.data(), src_data.data(), cnt);
    relative_consumer_sequence += cnt;
    return cnt;
}
