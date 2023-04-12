#include "slag/message_fragment.h"
#include "slag/message_record_fragment.h"
#include "slag/message_buffer_fragment.h"
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

        writer.write_unchecked(fragment.data());
    }

}
