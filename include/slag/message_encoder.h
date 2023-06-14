#pragma once

#include <span>
#include <vector>
#include <cstdint>
#include <cstddef>
#include "../../generated/record.h"
#include "transform.h"
#include "buffer_writer.h"

namespace slag {

    static constexpr size_t MAX_MESSAGE_SIZE          = 1ull << 20;
    static constexpr size_t MAX_MESSAGE_FIELD_COUNT   = 1ull << 12;
    static constexpr size_t MAX_MESSAGE_BITFIELD_SIZE = 8;

    struct MessageHeader {
        uint32_t length      : 20; // 1MB
        uint32_t field_count : 12; // 4096 fields
    };

    // An intermediate representation of the encoded message.
    class MessageState {
    public:
        [[nodiscard]] std::span<const uint64_t> fields() const;
        [[nodiscard]] std::span<const std::byte> appendage() const;

        void reset();

    private:
        friend class RecordEncoder;
        friend class MessageDecoder;

        size_t add_field(uint64_t field);
        void add_flag(bool value);
        void add_appendage(std::span<const std::byte> buffer);
        void add_appendage(std::string_view buffer);

    private:
        struct BitmaskFieldInfo {
            size_t index = 0;
            size_t width = 0;
        };

        std::vector<uint64_t>           fields_;
        std::vector<std::byte>          appendage_;
        std::optional<BitmaskFieldInfo> bitmask_field_info_;
    };

    class MessageEncoder {
    public:
        MessageEncoder();

        template<RecordType type>
        [[nodiscard]] size_t encode(const Record<type>& record, std::span<std::byte> output_buffer);

        void reset();

    public:
        void enter(bool value);
        void enter(std::byte value);
        void enter(int8_t value);
        void enter(int16_t value);
        void enter(int32_t value);
        void enter(int64_t value);
        void enter(uint8_t value);
        void enter(uint16_t value);
        void enter(uint32_t value);
        void enter(uint64_t value);
        void enter(const std::string& value);

        template<typename T>
        void enter(const std::optional<T>& value);

        template<typename... Types>
        void enter(const std::pair<Types...>& value);

        template<typename... Types>
        void enter(const std::tuple<Types...>& value);

        template<typename... Types>
        void enter(const std::variant<Types...>& value);

        void enter(const std::monostate& value);

        void enter(const std::vector<std::byte>& value);

        template<typename T>
        void enter(const std::vector<T>& value);

        template<typename Key, typename T>
        void enter(const std::unordered_map<Key, T>& value);

        template<RecordType type>
        void enter(const Record<type>& value);

        template<RecordType type>
        void leave(const Record<type>& value);

        template<typename T>
        void enter(const T& value);

        template<typename T>
        void leave(const T& value);

    private:
        size_t add_field(uint64_t field);
        void add_flag(bool value);

        [[nodiscard]] size_t encode(std::span<std::byte> output_buffer);

    private:
        struct BitmaskFieldInfo {
            size_t index = 0;
            size_t width = 0;
        };

        std::vector<uint64_t>           fields_;
        std::vector<std::byte>          appendage_;
        std::optional<BitmaskFieldInfo> bitmask_field_info_;
    };

}
