#include <cstring>
#include <cassert>

namespace slag {

    inline BufferReader::BufferReader(std::span<const std::byte> buffer)
        : buffer_{buffer}
        , offset_{0}
    {
    }

    inline std::span<const std::byte> BufferReader::buffer() const {
        return buffer_;
    }

    inline size_t BufferReader::offset() const {
        return offset_;
    }

    inline size_t BufferReader::readable_size_bytes() const {
        return buffer_.size_bytes() - offset_;
    }

    inline std::optional<std::byte> BufferReader::read() {
        if (readable_size_bytes() < sizeof(std::byte)) {
            return std::nullopt;
        }

        return read_unchecked();
    }

    inline std::optional<std::span<const std::byte>> BufferReader::read(size_t size_bytes) {
        if (readable_size_bytes() < size_bytes) {
            return std::nullopt;
        }

        return read_unchecked(size_bytes);
    }

    inline std::byte BufferReader::read_unchecked() {
        return buffer_[offset_++];
    }

    inline std::span<const std::byte> BufferReader::read_unchecked(size_t size_bytes) {
        auto result = std::span{&buffer_[offset_], size_bytes};
        offset_ += size_bytes;
        return result;
    }

}
