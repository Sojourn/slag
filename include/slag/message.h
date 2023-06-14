#pragma once

#include <string_view>
#include <optional>
#include <span>
#include <vector>
#include <cstdint>
#include <cstddef>

namespace slag {

    static constexpr size_t MAX_MESSAGE_FLAG_SLOT_WIDTH = 8;

    struct MessageFlagSlot {
        size_t index = 0;
        size_t width = 0;
    };

    class Message {
        friend class MessageWriter;
        friend class MessageReader;

    public:
        [[nodiscard]] std::span<const uint64_t> slots() const;
        [[nodiscard]] std::span<const std::byte> appendage() const;

        void clear();

    private:
        std::vector<uint64_t>  slots_;
        std::vector<std::byte> appendage_;
    };

    class MessageWriter {
    public:
        explicit MessageWriter(Message& message);

        void write_flag(bool flag);
        void write_slot(uint64_t slot);
        void write_text(std::string_view text);
        void write_blob(std::span<const std::byte> blob);

    private:
        Message&                       message_;
        std::optional<MessageFlagSlot> flag_slot_;
    };

    class MessageReader {
    public:
        explicit MessageReader(const Message& message);

        [[nodiscard]] bool read_flag();
        [[nodiscard]] uint64_t read_slot();
        [[nodiscard]] std::string_view read_text(size_t size);
        [[nodiscard]] std::span<const std::byte> read_blob(size_t size);

    private:
        const Message&                 message_;
        std::optional<MessageFlagSlot> flag_slot_;
        size_t                         slot_index_;
        size_t                         appendage_offset_;
    };

}
