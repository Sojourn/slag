#pragma once

#include <span>
#include <cstdint>
#include <cstddef>
#include "slag/buffer_writer.h"

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

    [[nodiscard]] void write_message_fragment(BufferWriter& writer, const MessageRecordFragment& fragment);
    [[nodiscard]] void write_message_fragment(BufferWriter& writer, const MessageBufferFragment& fragment);

    template<typename FragmentHandler>
    [[nodiscard]] size_t read_message_fragment(std::span<const std::byte> buffer, FragmentHandler&& handler);

}
