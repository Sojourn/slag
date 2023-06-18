#include "slag/record_encoder.h"
#include "slag/transform.h"
#include "slag/visit.h"
#include <type_traits>

namespace slag {

    class RecordEncoder {
    public:
        explicit RecordEncoder(Message& message)
            : writer_{message}
        {
        }

    public:
        void enter(bool value) {
            writer_.write_flag(value);
        }

        void enter(std::byte value) {
            (void)value;
        }

        void enter(int8_t value) {
            enter(int64_t{value});
        }

        void enter(int16_t value) {
            enter(int64_t{value});
        }

        void enter(int32_t value) {
            enter(int64_t{value});
        }

        void enter(int64_t value) {
            enter(encode_zig_zag(value));
        }

        void enter(uint8_t value) {
            enter(uint64_t{value});
        }

        void enter(uint16_t value) {
            enter(uint64_t{value});
        }

        void enter(uint32_t value) {
            enter(uint64_t{value});
        }

        void enter(uint64_t value) {
            writer_.write_slot(value);
        }

        void enter(const std::string& value) {
            writer_.write_text(value);
        }

        template<typename T>
        void enter(const std::optional<T>& value) {
            enter(static_cast<bool>(value));
        }

        template<typename... Types>
        void enter(const std::pair<Types...>& value) {
            (void)value;
        }

        template<typename... Types>
        void enter(const std::tuple<Types...>& value) {
            (void)value;
        }

        template<typename... Types>
        void enter(const std::variant<Types...>& value) {
            enter(value.index());
        }

        void enter(const std::monostate& value) {
            (void)value;
        }

        void enter(const std::vector<std::byte>& value) {
            writer_.write_blob(
                std::span{value.data(), value.size()}
            );
        }

        template<typename T>
        void enter(const std::vector<T>& value) {
            enter(value.size());
        }

        template<typename Key, typename T>
        void enter(const std::unordered_map<Key, T>& value) {
            enter(value.size());
        }

        template<RecordType type>
        void enter(const Record<type>& value) {
            (void)value;
        }

        template<RecordType type>
        void leave(const Record<type>& value) {
            (void)value;
        }

        template<typename T>
        void enter(const T& value) {
            static_assert(std::is_enum_v<T>, "Missing case for this type.");

            enter(static_cast<std::underlying_type_t<T>>(value));
        }

        template<typename T>
        void leave(const T& value) {
            (void)value;
        }

    private:
        MessageWriter writer_;
    };

    template<RecordType type>
    void encode(const Record<type>& record, Message& message) {
        RecordEncoder encoder{message};
        encoder.enter(static_cast<std::underlying_type_t<RecordType>>(type));
        visit(encoder, record);
    }

#define X(SLAG_RECORD_TYPE)                                                            \
    template                                                                           \
    void encode(const Record<RecordType::SLAG_RECORD_TYPE>& record, Message& message); \

    SLAG_RECORD_TYPES(X)
#undef X

}
