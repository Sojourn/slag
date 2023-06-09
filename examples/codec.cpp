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
    RecordEncoder() {
        fields_.reserve(16 * 1024);
        appendage_.reserve(16 * 1024);
        output_.resize(16 * 1024 * 1024);
    }

    template<RecordType type>
    [[nodiscard]] std::span<const std::byte> encode(const Record<type>& record) {
        reset();

        // populate fields_ and appendage_
        fields_.push_back(static_cast<size_t>(type));
        visit(*this, record);

        auto writer = BufferWriter {
            std::span {
                output_.data(),
                output_.size(),
            }
        };

        // write message fields
        MessageRecordFragment record_fragment;
        record_fragment.set_head();
        for (size_t i = 0; i < fields_.size(); ++i) {
            // check if we need to flush this fragment
            if (record_fragment.is_full()) {
                write_message_fragment(writer, record_fragment);
                record_fragment.clear();
            }

            record_fragment.push_back(fields_[i]);
        }

        // TODO: refactor this
        if (appendage_.empty()) {
            record_fragment.set_tail();
            write_message_fragment(writer, record_fragment);
        }
        else {
            write_message_fragment(writer, record_fragment);

            auto buffer = std::span {
                appendage_.data(),
                appendage_.size(),
            };

            MessageBufferFragment buffer_fragment;
            while (!buffer.empty()) {
                // check if we need to flush this fragment
                if (buffer_fragment.is_full()) {
                    write_message_fragment(writer, buffer_fragment);
                    buffer_fragment.clear();
                }

                // figure out how much of the source buffer we can pack into this fragment
                size_t length = std::min(buffer.size(), buffer_fragment.capacity());
                buffer_fragment.resize(length);

                auto src = buffer.first(length);
                auto dst = buffer_fragment.data();
                memcpy(dst.data(), src.data(), length);

                // slice off the bit we just copied
                buffer = buffer.subspan(length);
            }

            buffer_fragment.set_tail();
            write_message_fragment(writer, buffer_fragment);
        }

        // TODO: make this more ergonomic
        return writer.buffer().first(
            writer.offset()
        );
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
    // NOTE: bitfield size (1, 8, ...)  have some tradeoffs (how often it changes, present mask size)

    std::vector<uint64_t>  fields_;
    std::vector<std::byte> appendage_;
    std::vector<std::byte> output_;
};

// JSR: just thinking about how to frame a message
//
// could add a stream offset for the reference message
//
/*
struct MessageHeader {
    uint32_t message_length; // steal bits for flags from here
    uint16_t field_count;
    // uint16_t type;           // this can be a field (small, doesn't change)

    // present bitmask
    // encoding bitmask (less fields that aren't present)
    // encoded fields
    // appendage
};
*/

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    TestStruct test_struct;
    test_struct.a.push_back(0);
    test_struct.a.push_back(1);
    test_struct.a.push_back(2);
    test_struct.c = "hello";

    RecordEncoder encoder;
    std::span<const std::byte> buffer = encoder.encode(test_struct);
    std::cout << buffer.size() << std::endl;

    return 0;
}
