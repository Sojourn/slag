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

    template<RecordType type>
    struct Record;

    template<RecordType type>
    struct RecordInfo;

    template<RecordType type, size_t index>
    struct RecordFieldInfo;

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

    template<>
    struct RecordInfo<RecordType::HEADER> {
        constexpr static std::string_view type_name = to_string(RecordType::HEADER);
        constexpr static size_t field_count = 3;
    }

    template<>
    struct RecordFieldInfo<RecordType::HEADER, 0> {
        constexpr static std::string_view name = "channel";
        constexpr static std::string Record<RecordType::HEADER>::*accessor = &Record<RecordType::HEADER>::channel;
    }

    template<>
    struct RecordFieldInfo<RecordType::HEADER, 1> {
        constexpr static std::string_view name = "topic";
        constexpr static std::string Record<RecordType::HEADER>::*accessor = &Record<RecordType::HEADER>::topic;
    }

    template<>
    struct RecordFieldInfo<RecordType::HEADER, 2> {
        constexpr static std::string_view name = "sequence_number";
        constexpr static uint64_t Record<RecordType::HEADER>::*accessor = &Record<RecordType::HEADER>::sequence_number;
    }

    template<>
    struct RecordInfo<RecordType::TEST_STRUCT> {
        constexpr static std::string_view type_name = to_string(RecordType::TEST_STRUCT);
        constexpr static size_t field_count = 7;
    }

    template<>
    struct RecordFieldInfo<RecordType::TEST_STRUCT, 0> {
        constexpr static std::string_view name = "header";
        constexpr static Header Record<RecordType::TEST_STRUCT>::*accessor = &Record<RecordType::TEST_STRUCT>::header;
    }

    template<>
    struct RecordFieldInfo<RecordType::TEST_STRUCT, 1> {
        constexpr static std::string_view name = "a";
        constexpr static std::vector<int8_t> Record<RecordType::TEST_STRUCT>::*accessor = &Record<RecordType::TEST_STRUCT>::a;
    }

    template<>
    struct RecordFieldInfo<RecordType::TEST_STRUCT, 2> {
        constexpr static std::string_view name = "b";
        constexpr static bool Record<RecordType::TEST_STRUCT>::*accessor = &Record<RecordType::TEST_STRUCT>::b;
    }

    template<>
    struct RecordFieldInfo<RecordType::TEST_STRUCT, 3> {
        constexpr static std::string_view name = "c";
        constexpr static std::string Record<RecordType::TEST_STRUCT>::*accessor = &Record<RecordType::TEST_STRUCT>::c;
    }

    template<>
    struct RecordFieldInfo<RecordType::TEST_STRUCT, 4> {
        constexpr static std::string_view name = "d";
        constexpr static std::tuple<std::string, std::vector<uint8_t>> Record<RecordType::TEST_STRUCT>::*accessor = &Record<RecordType::TEST_STRUCT>::d;
    }

    template<>
    struct RecordFieldInfo<RecordType::TEST_STRUCT, 5> {
        constexpr static std::string_view name = "e";
        constexpr static std::variant<std::monostate, TestEnum, std::unordered_map<int8_t, int8_t>> Record<RecordType::TEST_STRUCT>::*accessor = &Record<RecordType::TEST_STRUCT>::e;
    }

    template<>
    struct RecordFieldInfo<RecordType::TEST_STRUCT, 6> {
        constexpr static std::string_view name = "f";
        constexpr static std::vector<std::byte> Record<RecordType::TEST_STRUCT>::*accessor = &Record<RecordType::TEST_STRUCT>::f;
    }

}

