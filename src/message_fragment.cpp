#include "slag/message_fragment.h"
#include "slag/message_record_fragment.h"
#include "slag/message_buffer_fragment.h"
#include <stdexcept>
#include <cassert>

namespace slag {

    struct alignas(16) FragmentEncodingContext {
        uint8_t data[MESSAGE_FRAGMENT_CAPACITY];
    };

    [[nodiscard]] FragmentEncodingContext make_record_encoding_context(const MessageRecordFragment& fragment) {
        constexpr const uint8_t table[8] = {
            8, 8, 8, 8,
            4, 4,
            2,
            1
        };

        FragmentEncodingContext context;
        memset(&context, 0, sizeof(context));
        for (size_t i = 0; i < fragment.size(); ++i) {
            uint64_t field = fragment[i];
            context.data[i] = table[__builtin_clzll(field | 1) >> 3];
        }

        return context;
    }

    uint64_t make_record_encoding_mask(const MessageRecordFragment& fragment, const FragmentEncodingContext& context) {
        uint64_t encoding_mask = 0;
        for (size_t i = 0; i < fragment.size(); ++i) {
            uint8_t size_bytes = context.data[i];
            encoding_mask |= __builtin_ctz(size_bytes) << (i * 2);
        }

        return encoding_mask;
    }

    void write_record_encoding_mask(BufferWriter& writer, uint64_t encoding_mask, size_t field_count) {
        auto saturated_encoding_mask = (1ull << (field_count * 2)) - 1;
        auto size_bytes              = 8 - (__builtin_clzll(saturated_encoding_mask) >> 3);

        switch (size_bytes) {
            case 8:
                writer.write_unchecked(static_cast<std::byte>((encoding_mask >> 56) & 0xff));
                [[fallthrough]];
            case 7:
                writer.write_unchecked(static_cast<std::byte>((encoding_mask >> 48) & 0xff));
                [[fallthrough]];
            case 6:
                writer.write_unchecked(static_cast<std::byte>((encoding_mask >> 40) & 0xff));
                [[fallthrough]];
            case 5:
                writer.write_unchecked(static_cast<std::byte>((encoding_mask >> 32) & 0xff));
                [[fallthrough]];
            case 4:
                writer.write_unchecked(static_cast<std::byte>((encoding_mask >> 24) & 0xff));
                [[fallthrough]];
            case 3:
                writer.write_unchecked(static_cast<std::byte>((encoding_mask >> 16) & 0xff));
                [[fallthrough]];
            case 2:
                writer.write_unchecked(static_cast<std::byte>((encoding_mask >>  8) & 0xff));
                [[fallthrough]];
            case 1:
                writer.write_unchecked(static_cast<std::byte>((encoding_mask >>  0) & 0xff));
                [[fallthrough]];
            case 0:
                break;
        }
    }

    void write_record_field(BufferWriter& writer, const MessageRecordFragment& fragment, const FragmentEncodingContext& context, size_t field_index) {
        auto field      = fragment[field_index];
        auto size_bytes = context.data[field_index];

        switch (size_bytes) {
            case 8:
                writer.write_unchecked(static_cast<std::byte>((field >> 56) & 0xff));
                writer.write_unchecked(static_cast<std::byte>((field >> 48) & 0xff));
                writer.write_unchecked(static_cast<std::byte>((field >> 40) & 0xff));
                writer.write_unchecked(static_cast<std::byte>((field >> 32) & 0xff));
                [[fallthrough]];
            case 4:
                writer.write_unchecked(static_cast<std::byte>((field >> 24) & 0xff));
                writer.write_unchecked(static_cast<std::byte>((field >> 16) & 0xff));
                [[fallthrough]];
            case 2:
                writer.write_unchecked(static_cast<std::byte>((field >>  8) & 0xff));
                [[fallthrough]];
            case 1:
                writer.write_unchecked(static_cast<std::byte>((field >>  0) & 0xff));
                [[fallthrough]];
            case 0:
                break;
        }
    }

    void write_message_fragment(BufferWriter& writer, const MessageRecordFragment& fragment) {
        if (writer.writable_size_bytes() < MAX_MESSAGE_RECORD_FRAGMENT_SIZE_BYTES) {
            throw std::runtime_error("Potential record fragment underflow");
        }

        auto context = make_record_encoding_context(fragment);
        {
            MessageFragmentHeader header;
            header.type = static_cast<uint8_t>(MessageFragmentType::RECORD);
            header.head = static_cast<uint8_t>(fragment.head());
            header.tail = static_cast<uint8_t>(fragment.tail());
            header.size = static_cast<uint8_t>(fragment.size());

            std::byte header_buffer[sizeof(header)];
            memcpy(header_buffer, &header, sizeof(header));
            writer.write_unchecked(std::span{header_buffer});
        }

        if (fragment.size()) {
            uint64_t encoding_mask = make_record_encoding_mask(fragment, context);
            write_record_encoding_mask(writer, encoding_mask, fragment.size());

            for (size_t i = 0; i < fragment.size(); ++i) {
                write_record_field(writer, fragment, context, i);
            }
        }
    }

    void write_message_fragment(BufferWriter& writer, const MessageBufferFragment& fragment) {
        if (writer.writable_size_bytes() < MAX_MESSAGE_BUFFER_FRAGMENT_SIZE_BYTES) {
            throw std::runtime_error("Potential buffer fragment underflow");
        }

        {
            MessageFragmentHeader header;
            header.type = static_cast<uint8_t>(MessageFragmentType::RECORD);
            header.head = static_cast<uint8_t>(fragment.head());
            header.tail = static_cast<uint8_t>(fragment.tail());
            header.size = static_cast<uint8_t>(fragment.size());

            std::byte header_buffer[sizeof(header)];
            memcpy(header_buffer, &header, sizeof(header));
            writer.write_unchecked(std::span{header_buffer});
        }

        if (fragment.size()) {
            writer.write_unchecked(fragment.data());
        }
    }

    std::optional<uint64_t> read_message_record_fragment_encoding_mask(BufferReader& reader, size_t field_count) {
        auto saturated_encoding_mask = (1ull << (field_count * 2)) - 1;
        auto size_bytes              = static_cast<size_t>(8 - (__builtin_clzll(saturated_encoding_mask) >> 3));

        if (reader.readable_size_bytes() < size_bytes) {
            return std::nullopt;
        }

        uint64_t result = 0;
        switch (size_bytes) {
            case 8:
                result |= static_cast<uint64_t>(reader.read_unchecked()) << 56;
                [[fallthrough]];
            case 7:
                result |= static_cast<uint64_t>(reader.read_unchecked()) << 48;
                [[fallthrough]];
            case 6:
                result |= static_cast<uint64_t>(reader.read_unchecked()) << 40;
                [[fallthrough]];
            case 5:
                result |= static_cast<uint64_t>(reader.read_unchecked()) << 32;
                [[fallthrough]];
            case 4:
                result |= static_cast<uint64_t>(reader.read_unchecked()) << 24;
                [[fallthrough]];
            case 3:
                result |= static_cast<uint64_t>(reader.read_unchecked()) << 16;
                [[fallthrough]];
            case 2:
                result |= static_cast<uint64_t>(reader.read_unchecked()) << 8;
                [[fallthrough]];
            case 1:
                result |= static_cast<uint64_t>(reader.read_unchecked()) << 0;
                [[fallthrough]];
            case 0:
                break;
        }

        return result;
    }

    // TODO: rename
    uint8_t encoded_field_mask(uint64_t encoding_mask, size_t field_index) {
        return static_cast<uint8_t>((encoding_mask >> (field_index * 2)) & 3);
    }

    // TODO: rename
    uint8_t encoded_field_size_bytes(uint8_t encoded_field_mask) {
        return static_cast<uint8_t>(1 << encoded_field_mask);
    }

    // TODO: rename
    uint8_t encoded_field_size_bytes(uint64_t encoding_mask, size_t field_index) {
        return encoded_field_size_bytes(encoded_field_mask(encoding_mask, field_index));
    }

    bool read_message_record_fragment(BufferReader& reader, MessageFragmentHandler& handler, const MessageFragmentHeader& header) {
        size_t field_count = header.size;

        MessageRecordFragment fragment;
        fragment.resize(field_count);
        if (header.head) {
            fragment.set_head();
        }
        if (header.tail) {
            fragment.set_tail();
        }

        auto encoding_mask = read_message_record_fragment_encoding_mask(reader, fragment.size());
        if (!encoding_mask) {
            return false; // missing data
        }

        // aggregate bounds checking for encoded fields
        size_t total_encoded_field_size_bytes = 0;
        for (size_t field_index = 0; field_index < field_count; ++field_index) {
            total_encoded_field_size_bytes += encoded_field_size_bytes(
                encoded_field_mask(*encoding_mask, field_index)
            );
        }
        if (reader.readable_size_bytes() < total_encoded_field_size_bytes) {
            return false; // missing data
        }

        for (size_t field_index = 0; field_index < field_count; ++field_index) {
            uint64_t& field = fragment[field_index];

            switch (encoded_field_mask(*encoding_mask, field_index)) {
                case 3: {
                    field |= static_cast<uint64_t>(reader.read_unchecked()) << 56;
                    field |= static_cast<uint64_t>(reader.read_unchecked()) << 48;
                    field |= static_cast<uint64_t>(reader.read_unchecked()) << 40;
                    field |= static_cast<uint64_t>(reader.read_unchecked()) << 32;
                    break;
                }
                case 2: {
                    field |= static_cast<uint64_t>(reader.read_unchecked()) << 24;
                    field |= static_cast<uint64_t>(reader.read_unchecked()) << 16;
                    break;
                }
                case 1: {
                    field |= static_cast<uint64_t>(reader.read_unchecked()) << 8;
                    break;
                }
                case 0: {
                    field |= static_cast<uint64_t>(reader.read_unchecked()) << 0;
                    break;
                }
            }
        }

        handler.handle_message_record_fragment(fragment);
        return true;
    }

    bool read_message_buffer_fragment(BufferReader& reader, MessageFragmentHandler& handler, const MessageFragmentHeader& header) {
        size_t buffer_size_bytes = header.size;

        MessageBufferFragment fragment;
        fragment.resize_uninitialized(buffer_size_bytes);
        if (header.head) {
            fragment.set_head();
        }
        if (header.tail) {
            fragment.set_tail();
        }

        if (reader.readable_size_bytes() < buffer_size_bytes) {
            return false; // missing data
        }

        {
            auto src_buf = reader.read_unchecked(buffer_size_bytes);
            auto dst_buf = fragment.data();

            assert(src_buf.size_bytes() == buffer_size_bytes);
            assert(dst_buf.size_bytes() == buffer_size_bytes);

            memcpy(dst_buf.data(), src_buf.data(), buffer_size_bytes);
        }

        handler.handle_message_buffer_fragment(fragment);
        return true;
    }

    std::optional<MessageFragmentHeader> read_message_fragment_header(BufferReader& reader) {
        MessageFragmentHeader header;

        auto buffer = reader.read(sizeof(header));
        if (!buffer) {
            return std::nullopt; // missing data
        }

        assert(sizeof(header) == buffer->size_bytes());
        memcpy(&header, buffer->data(), sizeof(header));
        return header;
    }

    bool read_message_fragment(BufferReader& reader, MessageFragmentHandler& handler) {
        BufferReader transaction = reader; // make a working copy of the reader

        auto header = read_message_fragment_header(transaction);
        if (!header) {
            return false;
        }

        bool result = false;
        switch (static_cast<MessageFragmentType>(header->type)) {
            case MessageFragmentType::RECORD: {
                result = read_message_record_fragment(transaction, handler, *header);
                break;
            }
            case MessageFragmentType::BUFFER: {
                result = read_message_buffer_fragment(transaction, handler, *header);
                break;
            }
        }
        if (result) {
            reader = transaction; // commit
        }

        return result;
    }

}
