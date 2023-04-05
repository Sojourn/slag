#include "slag/slag.h"
#include "slag/transform.h"
#include <vector>
#include <span>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/printf.h>

using namespace slag;

static constexpr size_t MAX_PACKET_SIZE              = 32 - 1;
static constexpr size_t MAX_RECORD_PACKET_SIZE_BYTES = 1 + 8 + (MAX_PACKET_SIZE * sizeof(uint64_t));
static constexpr size_t MAX_BUFFER_PACKET_SIZE_BYTES = 1 + MAX_PACKET_SIZE;

enum class PacketType : uint8_t {
    RECORD,
    BUFFER,
};

struct PacketHeader {
    uint8_t type : 1;
    uint8_t head : 1;
    uint8_t tail : 1;
    uint8_t size : 5;
};
static_assert(sizeof(PacketHeader) == 1);

enum class RecordFieldEncoding : uint8_t {
    ENCODE_U08 = 0,
    ENCODE_U16 = 1,
    ENCODE_U32 = 2,
    ENCODE_U64 = 3,
};

class RecordField {
public:
    RecordField()
        : value_{0}
    {
    }

    explicit RecordField(uint64_t value)
        : value_{value}
    {
    }

    [[nodiscard]] uint64_t value() const {
        return value_;
    }

    [[nodiscard]] size_t encoded_size_bytes() const {
        const uint8_t table[8] = {
            8,
            8,
            8,
            8,
            4,
            4,
            2,
            1,
        };

        return table[__builtin_clzll(value_ | 1) >> 3];
    }

    [[nodiscard]] RecordFieldEncoding encoding() const {
        switch (encoded_size_bytes()) {
            case 8: return RecordFieldEncoding::ENCODE_U64;
            case 4: return RecordFieldEncoding::ENCODE_U32;
            case 2: return RecordFieldEncoding::ENCODE_U16;
            case 1: return RecordFieldEncoding::ENCODE_U08;
        }

        __builtin_unreachable();
    }

    [[nodiscard]] std::pair<RecordFieldEncoding, size_t> encode(std::span<std::byte> buffer) const {
        auto encoding = RecordField::encoding();
        auto buffer_offset = size_t{0};
        switch (encoding) {
            case RecordFieldEncoding::ENCODE_U64: {
                buffer[buffer_offset++] = static_cast<std::byte>((value_ >> 56) & 0xFF);
                buffer[buffer_offset++] = static_cast<std::byte>((value_ >> 48) & 0xFF);
                buffer[buffer_offset++] = static_cast<std::byte>((value_ >> 40) & 0xFF);
                buffer[buffer_offset++] = static_cast<std::byte>((value_ >> 32) & 0xFF);
                [[fallthrough]];
            }
            case RecordFieldEncoding::ENCODE_U32: {
                buffer[buffer_offset++] = static_cast<std::byte>((value_ >> 24) & 0xFF);
                buffer[buffer_offset++] = static_cast<std::byte>((value_ >> 16) & 0xFF);
                [[fallthrough]];
            }
            case RecordFieldEncoding::ENCODE_U16: {
                buffer[buffer_offset++] = static_cast<std::byte>((value_ >> 8) & 0xFF);
                [[fallthrough]];
            }
            case RecordFieldEncoding::ENCODE_U08: {
                buffer[buffer_offset++] = static_cast<std::byte>((value_ >> 0) & 0xFF);
                break;
            }
        }

        return std::make_pair(encoding, buffer_offset);
    }

    [[nodiscard]] static std::pair<RecordField, size_t> decode(std::span<const std::byte> buffer, RecordFieldEncoding encoding) {
        auto value = uint64_t{0};
        auto buffer_offset = size_t{0};
        switch (encoding) {
            case RecordFieldEncoding::ENCODE_U64: {
                value |= static_cast<uint64_t>(buffer[buffer_offset++]) << 56;
                value |= static_cast<uint64_t>(buffer[buffer_offset++]) << 48;
                value |= static_cast<uint64_t>(buffer[buffer_offset++]) << 40;
                value |= static_cast<uint64_t>(buffer[buffer_offset++]) << 32;
                [[fallthrough]];
            }
            case RecordFieldEncoding::ENCODE_U32: {
                value |= static_cast<uint64_t>(buffer[buffer_offset++]) << 24;
                value |= static_cast<uint64_t>(buffer[buffer_offset++]) << 16;
                [[fallthrough]];
            }
            case RecordFieldEncoding::ENCODE_U16: {
                value |= static_cast<uint64_t>(buffer[buffer_offset++]) << 8;
                [[fallthrough]];
            }
            case RecordFieldEncoding::ENCODE_U08: {
                value |= static_cast<uint64_t>(buffer[buffer_offset++]) << 0;
                break;
            }
        }

        return std::pair(RecordField{value}, buffer_offset);
    }

private:
    uint64_t value_;
};

class Record {
public:
    Record()
        : size_{0}
        , head_{false}
        , tail_{false}
    {
    }

    explicit Record(size_t size)
        : size_{static_cast<uint8_t>(size)}
        , head_{false}
        , tail_{false}
    {
    }

    size_t size() const {
        return size_;
    }

    size_t capacity() const {
        return MAX_PACKET_SIZE;
    }

    size_t encode(std::span<std::byte> buffer) const {
        assert(MAX_RECORD_PACKET_SIZE_BYTES <= buffer.size_bytes());

        size_t buffer_offset = 0;

        auto header = PacketHeader {
            .type = static_cast<uint8_t>(PacketType::RECORD),
            .head = static_cast<uint8_t>(head_),
            .tail = static_cast<uint8_t>(tail_),
            .size = size_,
        };

        memcpy(&buffer[buffer_offset], &header, sizeof(header));
        buffer_offset += sizeof(header);

        size_t encoding_mask_size = encoding_mask_size_bytes();
        buffer_offset += encoding_mask_size;

        uint64_t encoding_mask = 0;
        for (uint8_t i = 0; i < size_; ++i) {
            RecordField field{field_values_[i]};

            auto&& [encoding, encoded_size_bytes] = field.encode(buffer.subspan(buffer_offset));
            encoding_mask |= (static_cast<uint64_t>(encoding) << (i * 2));
            buffer_offset += encoded_size_bytes;
        }

        // FIXME: make this big-endian
        switch (encoding_mask_size) {
            case 8: {
                buffer[1 + 7] = static_cast<std::byte>((encoding_mask >> 56) & 255);
                [[fallthrough]];
            }
            case 7: {
                buffer[1 + 6] = static_cast<std::byte>((encoding_mask >> 48) & 255);
                [[fallthrough]];
            }
            case 6: {
                buffer[1 + 5] = static_cast<std::byte>((encoding_mask >> 40) & 255);
                [[fallthrough]];
            }
            case 5: {
                buffer[1 + 4] = static_cast<std::byte>((encoding_mask >> 32) & 255);
                [[fallthrough]];
            }
            case 4: {
                buffer[1 + 3] = static_cast<std::byte>((encoding_mask >> 24) & 255);
                [[fallthrough]];
            }
            case 3: {
                buffer[1 + 2] = static_cast<std::byte>((encoding_mask >> 16) & 255);
                [[fallthrough]];
            }
            case 2: {
                buffer[1 + 1] = static_cast<std::byte>((encoding_mask >> 8) & 255);
                [[fallthrough]];
            }
            case 1: {
                buffer[1 + 0] = static_cast<std::byte>((encoding_mask >> 0) & 255);
                [[fallthrough]];
            }
            case 0: {
                break;
            }
        }

        return buffer_offset;
    }

    static std::pair<Record, size_t> decode(std::span<const std::byte> buffer) {
        auto buffer_offset = size_t{0};

        PacketHeader header;
        memcpy(&header, &buffer[buffer_offset], sizeof(header));
        buffer_offset += sizeof(header);

        assert(static_cast<PacketType>(header.type) == PacketType::RECORD);

        Record record;
        record.resize_uninitialized(header.size);
        if (header.head) {
            record.set_head();
        }
        if (header.tail) {
            record.set_tail();
        }

        auto encoding_mask      = uint64_t{0};
        auto encoding_mask_size = record.encoding_mask_size_bytes();
        switch (encoding_mask_size) {
            case 8: {
                encoding_mask |= static_cast<uint64_t>(buffer[buffer_offset + 7]) << 56;
                [[fallthrough]];
            }
            case 7: {
                encoding_mask |= static_cast<uint64_t>(buffer[buffer_offset + 6]) << 48;
                [[fallthrough]];
            }
            case 6: {
                encoding_mask |= static_cast<uint64_t>(buffer[buffer_offset + 5]) << 40;
                [[fallthrough]];
            }
            case 5: {
                encoding_mask |= static_cast<uint64_t>(buffer[buffer_offset + 4]) << 32;
                [[fallthrough]];
            }
            case 4: {
                encoding_mask |= static_cast<uint64_t>(buffer[buffer_offset + 3]) << 24;
                [[fallthrough]];
            }
            case 3: {
                encoding_mask |= static_cast<uint64_t>(buffer[buffer_offset + 2]) << 16;
                [[fallthrough]];
            }
            case 2: {
                encoding_mask |= static_cast<uint64_t>(buffer[buffer_offset + 1]) << 8;
                [[fallthrough]];
            }
            case 1: {
                encoding_mask |= static_cast<uint64_t>(buffer[buffer_offset + 0]) << 0;
                [[fallthrough]];
            }
            case 0: {
                break;
            }
        }
        buffer_offset += encoding_mask_size;

        for (size_t i = 0; i < record.size(); ++i) {
            auto&& encoding = static_cast<RecordFieldEncoding>((encoding_mask >> (2 * i)) & 3);
            auto&& [field, size_bytes] = RecordField::decode(buffer.subspan(buffer_offset), encoding);

            record[i] = field.value();
            buffer_offset += size_bytes;
        }

        return std::make_pair(record, buffer_offset);
    }

    void resize(size_t size) {
        if (size > MAX_PACKET_SIZE) {
            throw std::runtime_error("Record overflow");
        }

        // zero initialize new fields
        for (size_t i = size_; i < size; ++i) {
            field_values_[i] = 0;
        }

        size_ = static_cast<uint8_t>(size);
    }

    void resize_uninitialized(size_t size) {
        if (size > MAX_PACKET_SIZE) {
            throw std::runtime_error("Record overflow");
        }

        size_ = static_cast<uint8_t>(size);
    }

    void push_back(uint64_t value) {
        if (size() == capacity()) {
            throw std::runtime_error("Record overflow");
        }

        field_values_[size_++] = value;
    }

    uint64_t& operator[](size_t index) {
        assert(index < size_);

        return field_values_[index];
    }

    uint64_t operator[](size_t index) const {
        assert(index < size_);

        return field_values_[index];
    }

    [[nodiscard]] bool head() const {
        return head_;
    }

    [[nodiscard]] bool tail() const {
        return tail_;
    }

    void set_head(bool value = true) {
        head_ = value;
    }

    void set_tail(bool value = true) {
        tail_ = value;
    }

    friend bool operator==(const Record& l, const Record& r) {
        if (l.size() != r.size()) {
            return false;
        }
        if (l.head() != r.head()) {
            return false;
        }
        if (l.tail() != r.tail()) {
            return false;
        }

        for (size_t i = 0; i < l.size(); ++i) {
            if (l[i] != r[i]) {
                return false;
            }
        }

        return true;
    }

private:
    // returns the number of bytes needed to encode a saturated encoding mask for the current number of fields
    [[nodiscard]] size_t encoding_mask_size_bytes() const {
        if (size_) {
            size_t saturated_encoding_mask = (1ull << (size_ * 2)) - 1;
            return 8 - (__builtin_clzll(saturated_encoding_mask) >> 3);
        }

        return 0;
    }

private:
    uint8_t  size_;
    bool     head_;
    bool     tail_;
    uint64_t field_values_[MAX_PACKET_SIZE];
};

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    for (size_t j = 0; j < 100; ++j) {
        std::byte buffer[MAX_RECORD_PACKET_SIZE_BYTES];
        memset(buffer, 0, sizeof(buffer));

        Record src_record;
        for (size_t i = 0; i < src_record.capacity(); ++i) {
            src_record.push_back(
                static_cast<uint64_t>(i)
            );
        }

        size_t encoded_size = src_record.encode(std::span{buffer});
        std::cout << encoded_size << std::endl;

        auto&& [dst_record, _] = Record::decode(std::span{buffer});

        assert(src_record == dst_record);
    }

    return 0;
}
