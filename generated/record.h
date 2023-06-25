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
    X(LOGIN_REQUEST) \
    X(TEST_STRUCT) \

namespace slag {

    enum class TestEnum : uint8_t {
        FOO,
        BAR,
        BAZ,
    };

    enum class ModuleType : uint16_t {
        SESSION,
    };

    enum class RecordType : uint16_t {
        HEADER,
        LOGIN_REQUEST,
        TEST_STRUCT,
    };

    [[nodiscard]] std::optional<std::string_view> to_string(TestEnum value);
    [[nodiscard]] std::optional<std::string_view> to_string(ModuleType value);
    [[nodiscard]] std::optional<std::string_view> to_string(RecordType value);

    template<RecordType type>
    struct Record;

    using Header = Record<RecordType::HEADER>;
    using LoginRequest = Record<RecordType::LOGIN_REQUEST>;
    using TestStruct = Record<RecordType::TEST_STRUCT>;

    template<>
    struct Record<RecordType::HEADER> {
        uint64_t sequence_number = {};
    };

    template<>
    struct Record<RecordType::LOGIN_REQUEST> {
        Header header = {};
        std::string client_name = {};
        std::string server_name = {};
    };

    template<>
    struct Record<RecordType::TEST_STRUCT> {
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
            visit(visitor, record.sequence_number);
        }
        visitor.leave(record);
    }

    template<typename Visitor>
    constexpr inline void visit(Visitor&& visitor, const Header& record) {
        visitor.enter(record);
        {
            visit(visitor, record.sequence_number);
        }
        visitor.leave(record);
    }

    template<typename Visitor>
    constexpr inline void visit(Visitor&& visitor, LoginRequest& record) {
        visitor.enter(record);
        {
            visit(visitor, record.header);
            visit(visitor, record.client_name);
            visit(visitor, record.server_name);
        }
        visitor.leave(record);
    }

    template<typename Visitor>
    constexpr inline void visit(Visitor&& visitor, const LoginRequest& record) {
        visitor.enter(record);
        {
            visit(visitor, record.header);
            visit(visitor, record.client_name);
            visit(visitor, record.server_name);
        }
        visitor.leave(record);
    }

    template<typename Visitor>
    constexpr inline void visit(Visitor&& visitor, TestStruct& record) {
        visitor.enter(record);
        {
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

