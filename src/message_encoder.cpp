#include "slag/message_encoder.h"
#include "slag/visit.h"

namespace slag {

#if 0

    template<RecordType type>
    void MessageEncoder::enter(const Record<type>&) {
        // TODO: record the start of record field index
    }

    template<RecordType type>
    void MessageEncoder::leave(const Record<type>&) {
        // TODO: record the end of record field index
    }

    void MessageEncoder::enter(bool value) {
        add_flag(value);
    }

    void MessageEncoder::enter(std::byte) {
        // We are visiting elements of a blob, which we
        // have already copied into the appendage.
    }

    void MessageEncoder::enter(int8_t value) {
        enter(int64_t{value});
    }

    void MessageEncoder::enter(int16_t value) {
        enter(int64_t{value});
    }

    void MessageEncoder::enter(int32_t value) {
        enter(int64_t{value});
    }

    void MessageEncoder::enter(int64_t value) {
        enter(encode_zig_zag(value));
    }

    void MessageEncoder::enter(uint8_t value) {
        enter(uint64_t{value});
    }

    void MessageEncoder::enter(uint16_t value) {
        enter(uint64_t{value});
    }

    void MessageEncoder::enter(uint32_t value) {
        enter(uint64_t{value});
    }

    void MessageEncoder::enter(uint64_t value) {
        add_field(value);
    }

    void MessageEncoder::enter(const std::string& value) {
        auto buffer = std::as_bytes(std::span{value.data(), value.size()});

        add_field(value.size());
        appendage_.insert(appendage_.end(), buffer.begin(), buffer.end());
    }

    template<typename T>
    void MessageEncoder::enter(const std::optional<T>& value) {
        enter(static_cast<bool>(value));
    }

    template<typename... Types>
    void MessageEncoder::enter(const std::pair<Types...>&) {
        // pass
    }

    template<typename... Types>
    void MessageEncoder::enter(const std::tuple<Types...>&) {
        // pass
    }

    template<typename... Types>
    void MessageEncoder::enter(const std::variant<Types...>& value) {
        enter(value.index());
    }

    void MessageEncoder::enter(const std::monostate&) {
        // pass
    }

    void MessageEncoder::enter(const std::vector<std::byte>& value) {
        enter(value.size());
        appendage_.insert(appendage_.end(), value.begin(), value.end());
    }

    template<typename T>
    void MessageEncoder::enter(const std::vector<T>& value) {
        enter(value.size());
    }

    template<typename Key, typename T>
    void MessageEncoder::enter(const std::unordered_map<Key, T>& value) {
        enter(value.size());
    }

    template<typename T>
    void MessageEncoder::enter(const T& value) {
        static_assert(std::is_enum_v<T>, "Missing case for this type.");

        enter(static_cast<std::underlying_type_t<T>>(value));
    }

    template<typename T>
    void MessageEncoder::leave(const T&) {
        // pass
    }

#define X(SLAG_RECORD_TYPE)                                                                                                \
    template                                                                                                               \
    void MessageEncoder::encode(const Record<RecordType::SLAG_RECORD_TYPE>& record, std::span<std::byte> output_buffer); \

    SLAG_RECORD_TYPES(X)
#undef X

#endif

}
