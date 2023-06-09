#include "slag/slag.h"
#include "slag/util.h"
#include "slag/visit.h"
#include "slag/transform.h"
#include "slag/buffer_writer.h"
#include "slag/message_fragment.h"
#include "slag/message_record_fragment.h"
#include "slag/message_buffer_fragment.h"
#include <vector>
#include <span>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/printf.h>
#include "test_generated.h"

using namespace slag;

class RecordEncoder {
public:
    template<RecordType type>
    [[nodiscard]] void encode(const Record<type>& record) {
        reset();

        visit(*this, record);
    }

    [[nodiscard]] std::span<const uint64_t> fields() const {
        return {
            fields_.data(),
            fields_.size(),
        };
    }

    [[nodiscard]] std::span<const std::byte> appendage() const {
        return {
            appendage_.data(),
            appendage_.size(),
        };
    }

    void reset() {
        fields_.clear();
        appendage_.clear();
    }

public:
    template<RecordType type>
    void enter(const Record<type>&) {
        // TODO: record the start of record field index
    }

    template<RecordType type>
    void leave(const Record<type>&) {
        // TODO: record the end of record field index
    }

    void enter(bool value) {
        enter(static_cast<uint64_t>(value));
    }

    void enter(std::byte) {
        // We are visiting elements of a blob, which we
        // have already copied into the appendage.
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
        fields_.push_back(value);
    }

    void enter(const std::string& value) {
        auto buffer = std::as_bytes(std::span{value.data(), value.size()});

        fields_.push_back(value.size());
        appendage_.insert(appendage_.end(), buffer.begin(), buffer.end());
    }

    template<typename T>
    void enter(const std::optional<T>& value) {
        enter(static_cast<bool>(value));
    }

    template<typename... Types>
    void enter(const std::pair<Types...>&) {
        // pass
    }

    template<typename... Types>
    void enter(const std::tuple<Types...>&) {
        // pass
    }

    template<typename... Types>
    void enter(const std::variant<Types...>& value) {
        enter(value.index());
    }

    void enter(const std::monostate&) {
        // pass
    }

    void enter(const std::vector<std::byte>& value) {
        enter(value.size());
        appendage_.insert(appendage_.end(), value.begin(), value.end());
    }

    template<typename T>
    void enter(const std::vector<T>& value) {
        enter(value.size());
    }

    template<typename Key, typename T>
    void enter(const std::unordered_map<Key, T>& value) {
        enter(value.size());
    }

    template<typename T>
    void enter(const T& value) {
        static_assert(std::is_enum_v<T>, "Missing case for this type.");

        enter(static_cast<std::underlying_type_t<T>>(value));
    }

    template<typename T>
    void leave(const T&) {
        // pass
    }

private:
    std::vector<uint64_t>  fields_;
    std::vector<std::byte> appendage_;
};

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    TestStruct test_struct;
    test_struct.a.push_back(0);
    test_struct.a.push_back(1);
    test_struct.a.push_back(2);
    test_struct.c = "hello";

    RecordEncoder encoder;
    encoder.encode(test_struct);

    return 0;
}
