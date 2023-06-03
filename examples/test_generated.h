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

    template<RecordType>
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
    constexpr inline void visit_fields(Header& object, Visitor&& visitor) {
        using namespace std::string_view_literals;

        visitor("channel"sv, object.channel);
        visitor("topic"sv, object.topic);
        visitor("sequence_number"sv, object.sequence_number);
    }

    template<typename Visitor>
    constexpr inline void visit_fields(TestStruct& object, Visitor&& visitor) {
        using namespace std::string_view_literals;

        visitor("header"sv, object.header);
        visitor("a"sv, object.a);
        visitor("b"sv, object.b);
        visitor("c"sv, object.c);
        visitor("d"sv, object.d);
        visitor("e"sv, object.e);
        visitor("f"sv, object.f);
    }

    template<typename Visitor>
    constexpr inline void visit_fields(const Header& object, Visitor&& visitor) {
        using namespace std::string_view_literals;

        visitor("channel"sv, object.channel);
        visitor("topic"sv, object.topic);
        visitor("sequence_number"sv, object.sequence_number);
    }

    template<typename Visitor>
    constexpr inline void visit_fields(const TestStruct& object, Visitor&& visitor) {
        using namespace std::string_view_literals;

        visitor("header"sv, object.header);
        visitor("a"sv, object.a);
        visitor("b"sv, object.b);
        visitor("c"sv, object.c);
        visitor("d"sv, object.d);
        visitor("e"sv, object.e);
        visitor("f"sv, object.f);
    }

}

