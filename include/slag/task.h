#pragma once

#include <optional>
#include <cstddef>
#include "slag/error.h"
#include "slag/result.h"
#include "slag/executor.h"

namespace slag {

    class Executor;

    // TODO: support moving
    // TODO: think about having this templated on a member function pointer
    class Task {
    public:
        Task(Executor& executor=local_executor());
        Task(Task&&) noexcept = delete;
        Task(const Task&) = delete;
        virtual ~Task();

        Task& operator=(Task&&) noexcept = delete;
        Task& operator=(const Task&) = delete;

        [[nodiscard]] bool is_scheduled() const;
        void schedule(TaskPriority priority=TaskPriority::NORMAL);
        void cancel();

        virtual void run() = 0;

    private:
        friend class Executor;

        // ???

    private:
        Executor&                                   executor_;
        std::optional<Executor::ScheduledTaskEntry> scheduled_task_entry_;
    };

}
