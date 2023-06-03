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

    [[nodiscard]] std::string_view to_string(TestEnum value);

    struct TestStruct;

    struct TestStruct {
        std::vector<int8_t> a = {};
        bool b = {};
        std::string c = {};
        std::tuple<std::string, std::vector<uint8_t>> d = {};
        std::variant<std::monostate, TestEnum, std::unordered_map<int8_t, int8_t>> e = {};
        std::vector<std::byte> f = {};
    };

    template<typename Visitor>
    constexpr inline void visit_fields(TestStruct& object, Visitor&& visitor) {
        using namespace std::string_view_literals;

        visitor("a"sv, object.a);
        visitor("b"sv, object.b);
        visitor("c"sv, object.c);
        visitor("d"sv, object.d);
        visitor("e"sv, object.e);
        visitor("f"sv, object.f);
    }

    template<typename Visitor>
    constexpr inline void visit_fields(const TestStruct& object, Visitor&& visitor) {
        using namespace std::string_view_literals;

        visitor("a"sv, object.a);
        visitor("b"sv, object.b);
        visitor("c"sv, object.c);
        visitor("d"sv, object.d);
        visitor("e"sv, object.e);
        visitor("f"sv, object.f);
    }

}

