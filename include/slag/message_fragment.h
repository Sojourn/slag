#pragma once

#include <span>
#include <cstdint>
#include <cstddef>
#include "slag/buffer_writer.h"
#include "slag/buffer_reader.h"

namespace slag {

    class MessageRecordFragment;
    class MessageBufferFragment;

    static constexpr size_t MESSAGE_FRAGMENT_CAPACITY = 31;
    static constexpr size_t MAX_MESSAGE_RECORD_FRAGMENT_SIZE_BYTES = 1 + 8 + (MESSAGE_FRAGMENT_CAPACITY * sizeof(uint64_t));
    static constexpr size_t MAX_MESSAGE_BUFFER_FRAGMENT_SIZE_BYTES = 1 + (MESSAGE_FRAGMENT_CAPACITY * sizeof(std::byte));

    enum class MessageFragmentType : uint8_t {
        RECORD = 0,
        BUFFER = 1,
    };

    struct MessageFragmentHeader {
        uint8_t type : 1;
        uint8_t head : 1;
        uint8_t tail : 1;
        uint8_t size : 5;
    };
    static_assert(sizeof(MessageFragmentHeader) == 1);

    class MessageFragmentHandler {
    public:
        virtual ~MessageFragmentHandler() = default;

        virtual void handle_message_record_fragment(const MessageRecordFragment& fragment) = 0;
        virtual void handle_message_buffer_fragment(const MessageBufferFragment& fragment) = 0;
    };

    void write_message_fragment(BufferWriter& writer, const MessageRecordFragment& fragment);
    void write_message_fragment(BufferWriter& writer, const MessageBufferFragment& fragment);
    [[nodiscard]] bool read_message_fragment(BufferReader& reader, MessageFragmentHandler& handler);

}
