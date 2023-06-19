#include "slag/record_decoder.h"
#include "slag/transform.h"
#include "slag/visit.h"
#include "slag/util.h"
#include <type_traits>

namespace slag {

    class RecordDecoder {
    public:
        explicit RecordDecoder(MessageReader& reader)
            : reader_{reader}
        {
        }

    public:
        void leave(bool& value) {
            value = reader_.read_flag();
        }

        void leave(std::byte& value) {
            (void)value; // std::vector<std::byte>
        }

        void leave(int8_t& value) {
            value = static_cast<int8_t>(decode_zig_zag(reader_.read_slot()));
        }

        void leave(int16_t& value) {
            value = static_cast<int16_t>(decode_zig_zag(reader_.read_slot()));
        }

        void leave(int32_t& value) {
            value = static_cast<int32_t>(decode_zig_zag(reader_.read_slot()));
        }

        void leave(int64_t& value) {
            value = decode_zig_zag(reader_.read_slot());
        }

        void leave(uint8_t& value) {
            value = static_cast<uint8_t>(reader_.read_slot());
        }

        void leave(uint16_t& value) {
            value = static_cast<uint16_t>(reader_.read_slot());
        }

        void leave(uint32_t& value) {
            value = static_cast<uint32_t>(reader_.read_slot());
        }

        void leave(uint64_t& value) {
            value = reader_.read_slot();
        }

        void leave(std::string& value) {
            value = static_cast<std::string>(
                reader_.read_text(
                    reader_.read_slot() // size
                )
            );
        }

        template<typename T>
        void leave(std::optional<T>& value) {
            bool contains_value = reader_.read_flag();
            if (contains_value) {
                visit(*this, value.emplace());
            }
        }

        template<typename... Types>
        void leave(std::pair<Types...>& value) {
            (void)value;
        }

        template<typename... Types>
        void leave(std::tuple<Types...>& value) {
            (void)value;
        }

        template<typename... Types>
        void leave(std::variant<Types...>& value) {
            size_t index = reader_.read_slot();
            if (!index) {
                return; // std::monostate
            }

            auto accept = [&]<size_t I>(IndexWrapper<I>) {
                using Type = std::variant_alternative_t<I, std::variant<Types...>>;

                if (index == I) {
                    visit(*this, value.template emplace<Type>());
                }
            };

            [&]<size_t... I>(std::index_sequence<I...>) {
                (accept(IndexWrapper<I>{}), ...);
            }(std::make_index_sequence<sizeof...(Types)>{});
        }

        void leave(std::monostate& value) {
            (void)value;
        }

        void leave(std::vector<std::byte>& value) {
            auto buffer = reader_.read_blob(
                reader_.read_slot() // size
            );

            value.insert(value.end(), buffer.begin(), buffer.end());
        }

        template<typename T>
        void leave(std::vector<T>& value) {
            size_t size = reader_.read_slot();
            value.reserve(size);
            for (size_t i = 0; i < size; ++i) {
                visit(*this, value.emplace_back());
            }
        }

        template<typename Key, typename T>
        void leave(std::unordered_map<Key, T>& value) {
            size_t size = reader_.read_slot();
            for (size_t i = 0; i < size; ++i) {
                std::pair<Key, T> entry;
                visit(*this, entry.first);
                visit(*this, entry.second);
                value.insert(std::move(entry));
            }
        }

        template<RecordType type>
        void leave(Record<type>& value) {
            (void)value;
        }

        template<typename T>
        void enter(T& value) {
            (void)value;
        }

        template<typename T>
        void leave(T& value) {
            static_assert(std::is_enum_v<T>, "Missing case for this type.");

            std::underlying_type_t<T> enum_value = {};
            visit(*this, enum_value);
            value = static_cast<T>(enum_value);
        }

        template<typename T>
        void enter(const T&) {
            assert(false); // unreachable (std::pair<const Key, T>)
        }

        template<typename T>
        void leave(const T&) {
            assert(false); // unreachable (std::pair<const Key, T>)
        }

    private:
        MessageReader reader_;
    };

    template<RecordType type>
    void decode(Record<type>& record, MessageReader& reader) {
        RecordDecoder decoder{reader};
        visit(decoder, record);
    }

#define X(SLAG_RECORD_TYPE)                                                           \
    template                                                                          \
    void decode(Record<RecordType::SLAG_RECORD_TYPE>& record, MessageReader& reader); \

    SLAG_RECORD_TYPES(X)
#undef X

}
