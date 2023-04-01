#include "slag/slag.h"
#include "slag/transform.h"
#include <vector>
#include <span>

using namespace slag;

static constexpr size_t MAX_PACKET_SIZE = 32;

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

    size_t encoded_size_bytes() const {
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

    RecordFieldEncoding encoding() const {
        switch (encoded_size_bytes()) {
            case 8: return RecordFieldEncoding::ENCODE_U64;
            case 4: return RecordFieldEncoding::ENCODE_U32;
            case 2: return RecordFieldEncoding::ENCODE_U16;
            case 1: return RecordFieldEncoding::ENCODE_U08;
        }

        __builtin_unreachable();
    }

    static RecordField decode(std::span<const std::byte> buffer, RecordFieldEncoding encoding) {
        auto value = uint64_t{0};
        auto buffer_offset = size_t{1};
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

        return RecordField{value};
    }

    RecordFieldEncoding encode(std::span<std::byte> buffer) const {
        auto encoding = RecordField::encoding();
        auto buffer_offset = size_t{1};
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

        return encoding;
    }

private:
    uint64_t value_;
};

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    RecordField old_field{188383889};

    std::byte buffer[16];
    RecordFieldEncoding encoding = old_field.encode(buffer);
    RecordField new_field = RecordField::decode(buffer, encoding);
    (void)new_field;

    asm("int $3");

    return 0;
}
