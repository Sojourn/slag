#include <stdexcept>
#include <cstring>
#include <cassert>

namespace slag {

    inline BufferWriter::BufferWriter(std::span<std::byte> buffer)
        : buffer_{buffer}
        , offset_{0}
    {
    }

    inline std::span<std::byte> BufferWriter::buffer() {
        return buffer_;
    }

    inline std::span<const std::byte> BufferWriter::buffer() const {
        return buffer_;
    }

    inline size_t BufferWriter::offset() const {
        return offset_;
    }

    inline size_t BufferWriter::writable_size_bytes() const {
        return buffer_.size_bytes() - offset_;
    }

    inline void BufferWriter::write(std::byte data) {
        if (writable_size_bytes() < sizeof(data)) {
            throw std::runtime_error("BufferWriter underflow");
        }

        write_unchecked(data);
    }

    inline void BufferWriter::write(std::span<const std::byte> data) {
        if (writable_size_bytes() < data.size_bytes()) {
            throw std::runtime_error("BufferWriter underflow");
        }

        write_unchecked(data);
    }

    inline void BufferWriter::write_unchecked(std::byte data) {
        buffer_[offset_] = data;
        offset_ += sizeof(data);
    }

    inline void BufferWriter::write_unchecked(std::span<const std::byte> data) {
        size_t length = data.size_bytes();
        memcpy(&buffer_[offset_], data.data(), length);
        offset_ += length;
    }

}
