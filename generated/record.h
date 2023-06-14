#pragma once

#include <string>
#include <string_view>
#include <tuple>
#include <vector>
#include <variant>
#include <optional>
#include <unordered_set>
#include <unordered_map>
#include <cstddef>
#include <cstdint>

#define SLAG_RECORD_TYPES(X) \
    X(HEADER) \
    X(TEST_STRUCT) \

namespace slag {

    enum class TestEnum : uint8_t {
        FOO,
        BAR,
        BAZ,
    };

    enum class RecordType : uint16_t {
        HEADER,
        TEST_STRUCT,
    };

    [[nodiscard]] std::optional<std::string_view> to_string(TestEnum value);
    [[nodiscard]] std::optional<std::string_view> to_string(RecordType value);

    template<RecordType type>
    struct Record;

    using Header = Record<RecordType::HEADER>;
    using TestStruct = Record<RecordType::TEST_STRUCT>;

    template<>
    struct Record<RecordType::HEADER> {
        std::string channel = {};
        std::string topic = {};
        uint64_t sequence_number = {};
    };

    template<>
    struct Record<RecordType::TEST_STRUCT> {
        Header header = {};
        std::vector<int8_t> a = {};
        bool b = {};
        std::string c = {};
        std::tuple<std::string, std::vector<uint8_t>> d = {};
        std::variant<std::monostate, TestEnum, std::unordered_map<int8_t, int8_t>> e = {};
        std::vector<std::byte> f = {};
    };

    template<typename Visitor>
    constexpr inline void visit(Visitor&& visitor, Header& record) {
        visitor.enter(record);
        {
            visit(visitor, record.channel);
            visit(visitor, record.topic);
            visit(visitor, record.sequence_number);
        }
        visitor.leave(record);
    }

    template<typename Visitor>
    constexpr inline void visit(Visitor&& visitor, const Header& record) {
        visitor.enter(record);
        {
            visit(visitor, record.channel);
            visit(visitor, record.topic);
            visit(visitor, record.sequence_number);
        }
        visitor.leave(record);
    }

    template<typename Visitor>
    constexpr inline void visit(Visitor&& visitor, TestStruct& record) {
        visitor.enter(record);
        {
            visit(visitor, record.header);
            visit(visitor, record.a);
            visit(visitor, record.b);
            visit(visitor, record.c);
            visit(visitor, record.d);
            visit(visitor, record.e);
            visit(visitor, record.f);
        }
        visitor.leave(record);
    }

    template<typename Visitor>
    constexpr inline void visit(Visitor&& visitor, const TestStruct& record) {
        visitor.enter(record);
        {
            visit(visitor, record.header);
            visit(visitor, record.a);
            visit(visitor, record.b);
            visit(visitor, record.c);
            visit(visitor, record.d);
            visit(visitor, record.e);
            visit(visitor, record.f);
        }
        visitor.leave(record);
    }

}

