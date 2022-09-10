#pragma once

#include <type_traits>
#include <cstddef>
#include <cstdint>
#include "slag/error.h"
#include "slag/result.h"

namespace slag {

    enum class TaskState {
        RUNNING,
        SUCCESS,
        FAILURE,
    };

    class Task {
    public:
        Task();
        Task(Task&&) noexcept = delete;
        Task(const Task&) = delete;
        virtual ~Task() = default;

        Task& operator=(Task&&) noexcept = delete;
        Task& operator=(const Task&) = delete;

        [[nodiscard]] TaskState state() const;
        [[nodiscard]] Error error() const;
        [[nodiscard]] Result<TaskState> run();

    private:
        friend class Executor;

        [[nodiscard]] bool scheduled() const;
        void set_scheduled(bool value);

    private:
        TaskState state_;
        Error     error_;
        bool      scheduled_;
    };

}
