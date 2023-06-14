#include "slag/message_encoder.h"
#include "slag/visit.h"

namespace slag {

    std::span<const uint64_t> MessageState::fields() const {
        return {
            fields_.data(),
            fields_.size(),
        };
    }

    std::span<const std::byte> MessageState::appendage() const {
        return {
            appendage_.data(),
            appendage_.size(),
        };
    }

    size_t MessageState::add_field(uint64_t field) {
        size_t index = fields_.size();
        fields_.push_back(field);
        return index;
    }

    void MessageState::add_flag(bool value) {
        if (bitmask_field_info_) {
            auto&& [index, width] = *bitmask_field_info_;

            fields_[index] |= (static_cast<uint64_t>(value) << (1ull * width));

            width += 1;
            if (width == 8) {
                bitmask_field_info_.reset();
            }
        }
        else {
            bitmask_field_info_.emplace(
                BitmaskFieldInfo {
                    .index = add_field(static_cast<uint64_t>(value)),
                    .width = 1,
                }
            );
        }
    }

    void MessageState::add_appendage(std::span<const std::byte> buffer) {
        appendage_.insert(appendage_.end(), buffer.begin(), buffer.end());
    }

    void MessageState::add_appendage(std::string_view buffer) {
        add_appendage(
            std::as_bytes(
                std::span{buffer.data(), buffer.size()}
            )
        );
    }

    void MessageState::reset() {
        fields_.clear();
        appendage_.clear();
        bitmask_field_info_.reset();
    }

    MessageEncoder::MessageEncoder() {
        fields_.reserve(100);
        appendage_.reserve(1000);
    }

    template<RecordType type>
    size_t MessageEncoder::encode(const Record<type>& record, std::span<std::byte> output_buffer) {
        reset();

        // populate fields and the appendage
        add_field(static_cast<uint64_t>(type));
        visit(*this, record);

        (void)output_buffer;
        return 0;
        // return encode(output_buffer);
    }

    void MessageEncoder::reset() {
        fields_.clear();
        appendage_.clear();
        bitmask_field_info_.reset();
    }

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

    size_t MessageEncoder::add_field(uint64_t field) {
        size_t index = fields_.size();
        fields_.push_back(field);
        return index;
    }

    void MessageEncoder::add_flag(bool value) {
        if (bitmask_field_info_) {
            auto&& [index, width] = *bitmask_field_info_;

            fields_[index] |= (static_cast<uint64_t>(value) << (1ull * width));

            width += 1;
            if (width == 8) {
                bitmask_field_info_.reset();
            }
        }
        else {
            bitmask_field_info_.emplace(
                BitmaskFieldInfo {
                    .index = add_field(static_cast<uint64_t>(value)),
                    .width = 1,
                }
            );
        }
    }

#define X(SLAG_RECORD_TYPE)                                                                                                \
    template                                                                                                               \
    size_t MessageEncoder::encode(const Record<RecordType::SLAG_RECORD_TYPE>& record, std::span<std::byte> output_buffer); \

    SLAG_RECORD_TYPES(X)
#undef X

}

