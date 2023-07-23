#include "record.h"

namespace slag {

    std::optional<std::string_view> to_string(TestEnum value) {
        using namespace std::string_view_literals;

        switch (value) {
            case TestEnum::FOO: return "FOO"sv;
            case TestEnum::BAR: return "BAR"sv;
            case TestEnum::BAZ: return "BAZ"sv;
        }

        return std::nullopt;
    }

    std::optional<std::string_view> to_string(WorkerThreadState value) {
        using namespace std::string_view_literals;

        switch (value) {
            case WorkerThreadState::IDLE: return "IDLE"sv;
            case WorkerThreadState::RUNNING: return "RUNNING"sv;
            case WorkerThreadState::STOPPED: return "STOPPED"sv;
        }

        return std::nullopt;
    }

    std::optional<std::string_view> to_string(WorkerThreadEvent value) {
        using namespace std::string_view_literals;

        switch (value) {
            case WorkerThreadEvent::START: return "START"sv;
            case WorkerThreadEvent::STOP: return "STOP"sv;
        }

        return std::nullopt;
    }

    std::optional<std::string_view> to_string(ModuleType value) {
        using namespace std::string_view_literals;

        switch (value) {
            case ModuleType::TEST: return "TEST"sv;
        }

        return std::nullopt;
    }

    std::optional<std::string_view> to_string(RecordType value) {
        using namespace std::string_view_literals;

        switch (value) {
            case RecordType::HEADER: return "HEADER"sv;
            case RecordType::TEST_STRUCT: return "TEST_STRUCT"sv;
            case RecordType::TRANSITION_REQUEST: return "TRANSITION_REQUEST"sv;
            case RecordType::TRANSITION_REPLY: return "TRANSITION_REPLY"sv;
            case RecordType::TICK_REQUEST: return "TICK_REQUEST"sv;
            case RecordType::TICK_REPLY: return "TICK_REPLY"sv;
            case RecordType::WORKER_THREAD_REQUEST: return "WORKER_THREAD_REQUEST"sv;
            case RecordType::WORKER_THREAD_REPLY: return "WORKER_THREAD_REPLY"sv;
        }

        return std::nullopt;
    }

}

