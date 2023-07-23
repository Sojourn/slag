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
    X(TRANSITION_REQUEST) \
    X(TRANSITION_REPLY) \
    X(TICK_REQUEST) \
    X(TICK_REPLY) \
    X(WORKER_THREAD_REQUEST) \
    X(WORKER_THREAD_REPLY) \

namespace slag {

    enum class TestEnum : uint8_t {
        FOO,
        BAR,
        BAZ,
    };

    enum class WorkerThreadState : uint8_t {
        IDLE,
        RUNNING,
        STOPPED,
    };

    enum class WorkerThreadEvent : uint8_t {
        START,
        STOP,
    };

    enum class ModuleType : uint16_t {
        TEST,
    };

    enum class RecordType : uint16_t {
        HEADER,
        TEST_STRUCT,
        TRANSITION_REQUEST,
        TRANSITION_REPLY,
        TICK_REQUEST,
        TICK_REPLY,
        WORKER_THREAD_REQUEST,
        WORKER_THREAD_REPLY,
    };

    [[nodiscard]] std::optional<std::string_view> to_string(TestEnum value);
    [[nodiscard]] std::optional<std::string_view> to_string(WorkerThreadState value);
    [[nodiscard]] std::optional<std::string_view> to_string(WorkerThreadEvent value);
    [[nodiscard]] std::optional<std::string_view> to_string(ModuleType value);
    [[nodiscard]] std::optional<std::string_view> to_string(RecordType value);

    template<RecordType type>
    struct Record;

    using Header = Record<RecordType::HEADER>;
    using TestStruct = Record<RecordType::TEST_STRUCT>;
    using TransitionRequest = Record<RecordType::TRANSITION_REQUEST>;
    using TransitionReply = Record<RecordType::TRANSITION_REPLY>;
    using TickRequest = Record<RecordType::TICK_REQUEST>;
    using TickReply = Record<RecordType::TICK_REPLY>;
    using WorkerThreadRequest = Record<RecordType::WORKER_THREAD_REQUEST>;
    using WorkerThreadReply = Record<RecordType::WORKER_THREAD_REPLY>;

    template<>
    struct Record<RecordType::HEADER> {
        std::string channel = {};
        std::string topic = {};
        uint64_t sequence_number = {};
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

    template<>
    struct Record<RecordType::TRANSITION_REQUEST> {
        WorkerThreadEvent event = {};
    };

    template<>
    struct Record<RecordType::TRANSITION_REPLY> {
        WorkerThreadState old_state = {};
        WorkerThreadState new_state = {};
    };

    template<>
    struct Record<RecordType::TICK_REQUEST> {
        uint64_t epoch = {};
    };

    template<>
    struct Record<RecordType::TICK_REPLY> {
    };

    template<>
    struct Record<RecordType::WORKER_THREAD_REQUEST> {
        uint64_t request_id = {};
        std::variant<std::monostate, TransitionRequest, TickRequest> content = {};
    };

    template<>
    struct Record<RecordType::WORKER_THREAD_REPLY> {
        uint64_t request_id = {};
        std::variant<std::monostate, TransitionReply, TickReply> content = {};
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

    template<typename Visitor>
    constexpr inline void visit(Visitor&& visitor, TransitionRequest& record) {
        visitor.enter(record);
        {
            visit(visitor, record.event);
        }
        visitor.leave(record);
    }

    template<typename Visitor>
    constexpr inline void visit(Visitor&& visitor, const TransitionRequest& record) {
        visitor.enter(record);
        {
            visit(visitor, record.event);
        }
        visitor.leave(record);
    }

    template<typename Visitor>
    constexpr inline void visit(Visitor&& visitor, TransitionReply& record) {
        visitor.enter(record);
        {
            visit(visitor, record.old_state);
            visit(visitor, record.new_state);
        }
        visitor.leave(record);
    }

    template<typename Visitor>
    constexpr inline void visit(Visitor&& visitor, const TransitionReply& record) {
        visitor.enter(record);
        {
            visit(visitor, record.old_state);
            visit(visitor, record.new_state);
        }
        visitor.leave(record);
    }

    template<typename Visitor>
    constexpr inline void visit(Visitor&& visitor, TickRequest& record) {
        visitor.enter(record);
        {
            visit(visitor, record.epoch);
        }
        visitor.leave(record);
    }

    template<typename Visitor>
    constexpr inline void visit(Visitor&& visitor, const TickRequest& record) {
        visitor.enter(record);
        {
            visit(visitor, record.epoch);
        }
        visitor.leave(record);
    }

    template<typename Visitor>
    constexpr inline void visit(Visitor&& visitor, TickReply& record) {
        visitor.enter(record);
        {
        }
        visitor.leave(record);
    }

    template<typename Visitor>
    constexpr inline void visit(Visitor&& visitor, const TickReply& record) {
        visitor.enter(record);
        {
        }
        visitor.leave(record);
    }

    template<typename Visitor>
    constexpr inline void visit(Visitor&& visitor, WorkerThreadRequest& record) {
        visitor.enter(record);
        {
            visit(visitor, record.request_id);
            visit(visitor, record.content);
        }
        visitor.leave(record);
    }

    template<typename Visitor>
    constexpr inline void visit(Visitor&& visitor, const WorkerThreadRequest& record) {
        visitor.enter(record);
        {
            visit(visitor, record.request_id);
            visit(visitor, record.content);
        }
        visitor.leave(record);
    }

    template<typename Visitor>
    constexpr inline void visit(Visitor&& visitor, WorkerThreadReply& record) {
        visitor.enter(record);
        {
            visit(visitor, record.request_id);
            visit(visitor, record.content);
        }
        visitor.leave(record);
    }

    template<typename Visitor>
    constexpr inline void visit(Visitor&& visitor, const WorkerThreadReply& record) {
        visitor.enter(record);
        {
            visit(visitor, record.request_id);
            visit(visitor, record.content);
        }
        visitor.leave(record);
    }

}

